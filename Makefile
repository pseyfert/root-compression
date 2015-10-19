CFLAGS = -Wall -pedantic -O2 -fPIC -ggdb -pthread -Wextra -Izopfli/src
CXXFLAGS = -Wall -pedantic -O2 -fPIC -ggdb -pthread -Wextra -Izopfli/src
LIBS = -lz -llzo2 -L$(shell root-config --libdir) -lCore -lpthread -Lzopfli -lzopfli -Lbrotli/enc -lenc -Lbrotli/dec -ldec
CXX=$(shell root-config --cc)
SOFLAGS = -shared -ggdb -Bdynamic
LDFLAGS=$(LIBS)

all: libLzoRoot.so zopfli brotli

clean:
	rm -f *~ libLzoRoot.so *.o lz4/*.o
	$(MAKE) -C zopfli clean
	$(MAKE) -C brotli/enc clean
	$(MAKE) -C brotli/dec clean

libLzoRoot.so: libZpfRoot.o libZipRoot.o libBroRoot.o libLzoRoot.o lz4/lz4.o
	$(CXX) -o $@ $(SOFLAGS) $^ $(LIBS)
libLzoRoot.o: libLzoRoot.c lz4/lz4.c

zopfli/libzopfli.so: zopfli

brotli/enc/libenc.so: brotli

brotli/dec/libdec.so: brotli

zopfli:
	$(MAKE) -C zopfli libzopfli

brotli:
	$(MAKE) -C brotli/enc
	$(MAKE) -C brotli/dec

.PHONY: zopfli brotli
