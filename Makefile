CFLAGS = -Wall -pedantic -O2 -fPIC -ggdb -pthread -Wextra -Izopfli/src
CXXFLAGS = -Wall -pedantic -O2 -fPIC -ggdb -pthread -Wextra -Izopfli/src
LIBS = -lz -llzo2 -L$(shell root-config --libdir) -lCore -lpthread -Lzopfli -lzopfli -Lbrotli/enc -lenc -Lbrotli/dec -ldec
CXX=$(shell root-config --cc)
SOFLAGS = -shared -ggdb -Bdynamic
LDFLAGS=$(LIBS)

all: libLzoRoot.so

clean:
	rm -f *~ libLzoRoot.so *.o lz4/*.o
	$(MAKE) -C zopfli clean
	$(MAKE) -C brotli/enc clean
	$(MAKE) -C brotli/dec clean

libLzoRoot.so: libZpfRoot.o libZipRoot.o libBroRoot.o libLzoRoot.o lz4/lz4.o zopfli brotli
	$(CXX) -o $@ $(SOFLAGS) $(filter %.o,$^) $(LIBS)
libLzoRoot.o: libLzoRoot.c lz4/lz4.c

# hack
zopfli/libzopfli.so: zopfli

# hack
brotli/enc/libenc.so: brotli

# hack
brotli/dec/libdec.so: brotli

zopfli:
	$(MAKE) -C zopfli libzopfli

brotli:
	$(MAKE) -C brotli/enc
	$(MAKE) -C brotli/dec

test: callgrind-write massif-write size callgrind-read massif-read realtime-write realtime-read

.PHONY: zopfli brotli test callgrind-write massif-write size callgrind-read massif-read realtime-write realtime-read

# dependency = test only once the library exists
callgrind-read: all
	$(MAKE) -C test callgrind-read
callgrind-write: all
	$(MAKE) -C test callgrind-write
massif-read: all
	$(MAKE) -C test massif-read
massif-write: all
	$(MAKE) -C test massif-write
realtime-read: all
	$(MAKE) -C test realtime-read
realtime-write: all
	$(MAKE) -C test realtime-write
size: all
	$(MAKE) -C test size

