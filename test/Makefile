#ROOT_ENV: aliased to export CPPFLAGS="`root-config --cflags` -march=native -g -Wextra -Wall -Wshadow" ; export LDFLAGS="`root-config --libs` -lRooStats -lRooFitCore -lRooFit -lMinuit -lFoam -lMathMore -lTMVA -lstdc++ -m64 -g -march=native"; export CPP=c++; export CXX=c++; export CC=c++

parent.h: org.root
	root -l -b -q generate.C

parent.C: org.root
	root -l -b -q generate.C

read: read.o parent.o
