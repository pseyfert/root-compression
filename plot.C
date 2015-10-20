#include <boost/assign.hpp>

std::vector<std::string> name;
std::vector<float> size;
std::vector<float> writetime;
//compression   size  cpu writetime (writing to /dev/null)   real writetime (writing to /dev/null)  read real   read user   read sys
//

bool read_or_write = false;

int getColour( const bool reset = false )
{
  static const std::vector<int> colours = boost::assign::list_of
    (kBlack)(kRed)(kGreen)(kBlue)(kMagenta)(kCyan)(kOrange)
    (kAzure)(kViolet)(kPink)(kYellow)(kSpring)(kGray)(kTeal)
    ;
  static std::vector<int>::const_iterator i = colours.begin();
  if ( reset || i == colours.end() )
  {
    //std::cerr << "Warning - Run out of colours ...." << std::endl;
    i = colours.begin();
  }
  return *(i++);
}

void clear() {
  name.clear();
  size.clear();
  writetime.clear();
}
void graph(bool first) {
  float* x = &writetime[0];
  float* y = &size[0];
  TGraph * points = new TGraph((Int_t)writetime.size(),(Float_t*)x,(Float_t*)y);
  points->GetXaxis()->SetTitle("write time");
  points->GetYaxis()->SetTitle("compressed size");
  points->SetTitle(name[0].c_str());
  points->SetFillColor(kWhite);
  points->SetLineColor(getColour());
  if (first) {
    points->SetLineColor(kWhite);
    points->Draw("Psame");
  } else {
    points->Draw("L");
    points->SetMarkerStyle(kDot);
  }
  clear();
  //latex();
}
void fillme(std::string a,float b,float c, float, float d) {
  name.push_back(a);
  size.push_back(b);
  if (read_or_write) {
    writetime.push_back(d);
  } else {
    writetime.push_back(c);
  }
}
float make_time(float m,float s,float mm) {
  return m*60+s+mm/1000;
}
void latex() {
  TLatex tl;
  tl.SetTextSize(0.03);
  for (int i = 0 ; i < name.size() ; ++i) {
    tl.DrawText(writetime[i],size[i],name[i].c_str());
  }
}
void plot() {
  TCanvas* c = new TCanvas();
  TH1F* h = new TH1F("h","h",100,0,400);
  h->Draw();
  if (read_or_write) {
    h->GetXaxis()->SetTitle("read time");
  } else {
    h->GetXaxis()->SetTitle("write time");
  }
  h->GetYaxis()->SetTitle("compressed size");
  h->GetYaxis()->SetRangeUser(0,100000000);
  fillme("uncompressed  ",98301741,8.480000    , 8.505877 , make_time(0,7,746));//  0:3,158  0:1,173
  graph(true);
  clear();
  fillme("zlib1         ",50702386,12.361379   ,12.668328 , make_time(1,41,200));//   1:19,909   0:18,122
  fillme("zlib2         ",53557704,12.5551995  ,12.867823 , make_time(1,46,283));//   1:23,220   0:18,356
  fillme("zlib3         ",53358437,13.367272   ,13.747677 , make_time(1,41,800));//   1:20,576   0:17,999
  fillme("zlib4         ",53118472,13.7626665  ,14.012825 , make_time(1,39,407));//   1:20,900   0:16,261
  fillme("zlib5         ",52944945,15.0117325  ,15.295023 , make_time(1,43,267));//   1:22,739   0:18,148
  fillme("zlib6         ",52781813,17.696705   ,17.918442 , make_time(1,40,466));//   1:20,799   0:17,354
  fillme("zlib7         ",52715088,19.727918   ,19.948602 , make_time(1,41,667));//   1:21,114   0:18,223
  fillme("zlib8         ",52523744,27.3695925  ,27.542458 , make_time(1,45,558));//   1:25,527   0:17,556
  fillme("zlib9         ",52432407,45.533126   ,45.818158 , make_time(1,46,371));//   1:26,090   0:18,245
  graph(false);
  clear();
  fillme("lzma1         ",42628549,32.3426545  ,32.759363 , make_time(3,50,171));//   3:30,445   0:17,127
  fillme("lzma2         ",42549993,37.3112175  ,38.375471 , make_time(3,47,824));//   3:27,755   0:17,275
  fillme("lzma3         ",42449017,48.0045555  ,51.375971 , make_time(3,50,364));//   3:30,664   0:16,991
  fillme("lzma4         ",40730826,80.322336   ,79.646583 , make_time(3,50,024));//   3:30,012   0:17,900
  fillme("lzma5         ",40603482,111.350889  ,111.06531 , make_time(3,52,248));//   3:33,040   0:16,831
  fillme("lzma6         ",40548574,119.172748  ,118.78374 , make_time(3,52,201));//   3:33,160   0:16,732
  fillme("lzma7         ",40549134,170.5177875 ,170.27588 , make_time(3,46,638));//   3:25,923   0:18,207
  fillme("lzma8         ",40549430,271.7142525 ,275.87949 , make_time(3,57,510));//   3:32,380   0:22,675
  fillme("lzma9         ",40549358,271.725829  ,275.86248 , make_time(3,55,778));//   3:30,657   0:21,957
  graph(false);
  clear();
  fillme("lzo21         ",69482926,11.109108   ,11.475773 , make_time(1,43,608));//   1:23,969   0:17,455
  fillme("lzo22         ",69140839,11.4039395  ,11.815982 , make_time(1,42,795));//   1:22,465   0:18,017
  fillme("lzo23         ",68833535,12.232586   ,12.697652 , make_time(1,44,807));//   1:24,425   0:17,972
  fillme("lzo24         ",68833321,12.6829575  ,13.129823 , make_time(1,39,339));//   1:20,309   0:16,899
  fillme("lzo25         ",68948664,13.8776835  ,14.317636 , make_time(1,42,344));//   1:23,248   0:16,218
  fillme("lzo26         ",68948628,16.7288105  ,17.15721  , make_time(1,35,212));//   1:15,212   0:17,803
  fillme("lzo27         ",60529547,18.794627   ,19.225784 , make_time(1,39,787));//   1:19,115   0:18,033
  fillme("lzo28         ",60529548,26.4207385  ,26.75368  , make_time(1,43,315));//   1:20,949   0:16,541
  fillme("lzo29         ",60529549,44.854537   ,45.309934 , make_time(1,42,821));//   1:21,202   0:18,082
  graph(false);
  clear();
  fillme("lz41          ",69880277,11.198606   ,11.596689 , make_time(1,36,798));//   1:19,136   0:15,538
  fillme("lz42          ",69880372,11.5799305  ,11.954078 , make_time(1,29,686));//   1:12,345   0:15,144
  fillme("lz43          ",69881117,12.2273335  ,12.706084 , make_time(1,29,230));//   1:11,885   0:15,173
  fillme("lz44          ",69879308,12.674206   ,13.147816 , make_time(1,32,224));//   1:13,977   0:16,086
  fillme("lz45          ",69881284,13.949104   ,14.392899 , make_time(1,25,840));//   1: 9,302   0:14,498
  fillme("lz46          ",69879649,16.655103   ,17.083233 , make_time(1,45,811));//   1:27,783   0:15,809
  fillme("lz47          ",69880224,18.803855   ,19.214423 , make_time(1,45,494));//   1:27,350   0:16,162
  fillme("lz48          ",69879210,26.587827   ,26.982769 , make_time(1,33,812));//   1:15,697   0:15,929
  fillme("lz49          ",69881288,44.795453   ,45.161847 , make_time(1,39,214));//   1:19,240   0:17,778
  graph(false);
  clear();
  fillme("zopflizlib1   ",48102021,11.176992   ,11.619977 , make_time(1,46,204));//   1:25,981   0:17,994
  fillme("zopflizlib2   ",48077761,11.52606    ,12.040563 , make_time(1,46,318));//   1:25,242   0:18,604
  fillme("zopflizlib3   ",48066074,12.1624285  ,12.564434 , make_time(1,43,240));//   1:23,713   0:17,251
  fillme("zopflizlib4   ",48064696,12.6454145  ,12.973423 , make_time(1,46,002));//   1:26,826   0:16,446
  fillme("zopflizlib5   ",48063024,13.867307   ,14.279144 , make_time(1,46,725));//   1:25,930   0:18,353
  fillme("zopflizlib6   ",48061891,16.719647   ,17.170662 , make_time(1,43,676));//   1:24,188   0:16,599
  fillme("zopflizlib7   ",48061181,18.90807    ,19.252964 , make_time(1,43,456));//   1:22,292   0:18,580
  fillme("zopflizlib8   ",48058418,26.4213415  ,26.818588 , make_time(1,44,047));//   1:25,665   0:16,170
  fillme("zopflizlib9   ",48052394,44.9004185  ,45.370068 , make_time(1,46,309));//   1:24,796   0:18,304
  graph(false);
  clear();
  fillme("brotli1       ",55093705,17.750000   ,17.785291 , make_time(2,29,983));//   2, 5,114   0,19,275
  fillme("brotli2       ",55194416,18.060000   ,18.100760 , make_time(2,19,527));//   2, 3,169   0,12,204
  fillme("brotli3       ",55282577,19.980000   ,20.020245 , make_time(2,16,476));//   2, 3,274   0,11,136
  fillme("brotli4       ",52693765,22.170000   ,22.223890 , make_time(2,17,248));//   2, 4,052   0,11,593
  fillme("brotli5       ",45120783,36.890000   ,36.992227 , make_time(2,49,328));//   2,35,301   0,12,370
  fillme("brotli6       ",45050878,36.720000   ,36.814792 , make_time(2,46,978));//   2,35,334   0,10,454
  fillme("brotli7       ",45014172,50.680000   ,50.819970 , make_time(2,46,504));//   2,34.862   0,10,111
  fillme("brotli8       ",44997822,53.190000   ,53.337658 , make_time(2,46,227));//   2,35,007   0, 9,710
  //fillme("brotli9       ",44873267,230.760000  ,231.564378,make_time( ,  ,   ));//
  graph(false);
  c->BuildLegend();
}
