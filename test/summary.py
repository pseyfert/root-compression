from ROOT import *

maxlevel = 4

alglookup = { 1: "ZLIB",
              #2: "LZMA",
              #4: "LZO",
              #5: "LZ4",
              #6: "Zopfli",
              #7: "Brotli" 
            }

import os, re

colour_counter = 0
colours = [ROOT.kBlack,
           ROOT.kRed,
           ROOT.kGreen,
           ROOT.kBlue,
           ROOT.kMagenta,
           ROOT.kCyan,
           ROOT.kOrange,
           ROOT.kAzure,
           ROOT.kViolet,
           ROOT.kPink,
           ROOT.kYellow,
           ROOT.kSpring,
           ROOT.kGray,
           ROOT.kTeal]

def getColour( reset = False ):
  global colour_counter
  if reset:
    colour_counter = 0
  try:
    retval = colours[colour_counter]
    colour_counter+=1
  except IndexError:
    colour_counter = 1
    return colours[0]
  else:
    return retval

def find_size(alg,level):
   filename = 'size.'+str(alg)+'.'+str(level)+'.root'
   if os.path.isfile(filename):
      return os.stat(filename).st_size
   raise ValueError('file not found: ' + filename)
find_size.__name__ = "compressed size"

def find_memread(alg,level):
   filename = 'massif-read.'+str(alg)+'.'+str(level)+'.out'
   if os.path.isfile(filename):
      f = open(filename)
      lines = f.readlines()
      try:
         ind = lines.index('heap_tree=peak\n')
         toread = ind-3
         peakmem = int(lines[toread].split('=')[1])
         return peakmem
      except ValueError:
         raise
   raise ValueError('file not found: ' + filename)
find_memread.__name__ = "memory reading (peak)"


def find_memwrite(alg,level):
   filename = 'massif-write.'+str(alg)+'.'+str(level)+'.out'
   if os.path.isfile(filename):
      f = open(filename)
      lines = f.readlines()
      try:
         ind = lines.index('heap_tree=peak\n')
         toread = ind-3
         peakmem = int(lines[toread].split('=')[1])
         return peakmem
      except ValueError:
         raise
   raise ValueError('file not found: ' + filename)
find_memwrite.__name__ = "memory writing (peak)"



def find_callwrite(alg,level):
   filename = 'callgrind-write.'+str(alg)+'.'+str(level)+'.out'
   if os.path.isfile(filename):
      f = open(filename)
      lines = f.readlines()
      try:
         ind = -1
         for l in lines:
             if re.match('.*R__zipMultipleAlgorithm.*',l):
                ind = lines.index(l)
         if ind < 0:
             raise ValueError("didn't find R__zipMultipleAlgorithm in "+filename)
         toread = ind+1
         pointer = lines[toread].split(' ')[0]
         for l in lines:
             if re.match('calls.* '+pointer+'.*',l):
                return int(lines[lines.index(l)+1].split(' ')[-1])
         raise ValueError('callcount not found')
      except ValueError:
         print "failure in find_callwrite, searching ",filename
         raise
   raise ValueError('file not found: ' + filename)
find_callwrite.__name__ = "cycles writing (function)"


import numpy

stats = [find_memwrite,find_size,find_callwrite]

graphmap = {}
for alg in alglookup:
   columns = []
   algresult = []
   for level in range(1,maxlevel):
       try:
          vals = []
	  for func in stats:
             vals.append(func(alg,level))
             #print 'executed ', func.__name__, ' for alg ', alg, ' at level ', level, '\tobtained ', vals[-1]
       except ValueError:
          print "couldn't determine all inputs for alg " + str(alg) + " at level " + str(level)
          #raise
       else:
          algresult.append(vals)
       #print 'vals ', vals
       #print 'algresult ', algresult
   from numpy import array
   #print 'algresult ', algresult
   algresults = array(algresult)
   #print 'algresults ', algresults
   for column in range(len(stats)):
      the_column = algresults[:,column]
      #print 'the_column ', the_column
      columns.append(the_column)
   graphmap[alg] = columns
   

#print graphmap
for xaxis in range(len(stats)-1):
   for yaxis in range(xaxis+1,len(stats)):
      canvas = TCanvas()
      first = True
      xmax = array([ graphmap[alg][xaxis].max() for alg in alglookup ]).max()
      ymax = array([ graphmap[alg][yaxis].max() for alg in alglookup ]).max()
      h = TH1F("h","h",1,0,xmax)
      h.GetYaxis().SetRangeUser(0,ymax*1.05)
      h.Draw()
      for alg in alglookup:
          x = graphmap[alg][xaxis]
          y = graphmap[alg][yaxis]
          #points = TGraph(len(x),x,y)
          points = TGraph(len(x))
          for i in range(len(x)):
             points.SetPoint(i,x[i],y[i])
          points.GetXaxis().SetTitle(stats[xaxis].__name__)
          points.GetYaxis().SetTitle(stats[yaxis].__name__)
          points.SetTitle(alglookup[alg])
          points.SetFillColor(kWhite)
          points.SetLineColor(getColour())
          points.SetMarkerColor(points.GetLineColor())
          if first :
              points.SetLineColor(kWhite)
              points.Draw("Psame")
          else :
              points.Draw("PL")
              points.SetMarkerStyle(ROOT.kPlus)##kDot)
          first = False
      canvas.BuildLegend().SetFillColor(ROOT.kWhite)
      canvas.BuildLegend().SetFillStyle(0)
      canvas.SaveAs("plot_"+str(xaxis)+"_"+str(yaxis)+".png")
      canvas.SaveAs("plot_"+str(xaxis)+"_"+str(yaxis)+".eps")
      canvas.SaveAs("plot_"+str(xaxis)+"_"+str(yaxis)+".pdf")
      canvas.SaveAs("plot_"+str(xaxis)+"_"+str(yaxis)+".root")
      canvas.SaveAs("plot_"+str(xaxis)+"_"+str(yaxis)+".C")
      del h
