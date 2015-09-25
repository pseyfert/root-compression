CFLAGS = -Wall -pedantic -O2 -fPIC -g -pthread -Wextra
LIBS = -lz -llzo2 -L$(shell root-config --libdir) -lCore -lpthread
CXX=$(shell root-config --cc)
SOFLAGS = -shared -g -Bdynamic

all: libLzoRoot.so

clean:
	rm -f *~ libLzoRoot.so *.o lz4/*.o

libLzoRoot.so: libZipRoot.o libLzoRoot.o lz4/lz4.o
	$(CXX) -o $@ $(SOFLAGS) $^ $(LIBS)
libLzoRoot.o: libLzoRoot.c lz4/lz4.c
