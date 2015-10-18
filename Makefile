CFLAGS = -Wall -pedantic -O2 -fPIC -ggdb -pthread -Wextra -Izopfli/src
LIBS = -lz -llzo2 -L$(shell root-config --libdir) -lCore -lpthread -Lzopfli -lzopfli
CXX=$(shell root-config --cc)
SOFLAGS = -shared -ggdb -Bdynamic
LDFLAGS=$(LIBS)

all: libLzoRoot.so

clean:
	rm -f *~ libLzoRoot.so *.o lz4/*.o
	$(MAKE) -C zopfli clean

libLzoRoot.so: libZpfRoot.o libZipRoot.o libLzoRoot.o lz4/lz4.o zopfli/libzopfli.so
	$(CXX) -o $@ $(SOFLAGS) $^ $(LIBS)
libLzoRoot.o: libLzoRoot.c lz4/lz4.c

zopfli/libzopfli.so: zopfli

zopfli:
	$(MAKE) -C zopfli libzopfli

brotli/install/lib/python2.7/site-packages/brotli.so: brotli

brotli:
	$(MAKE) -C brotli/enc
	$(MAKE) -C brotli/dec

.PHONY: zopfli brotli
