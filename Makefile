CFLAGS = -Wall -pedantic -O2 -fPIC -g -pthread -Wextra -Izopfli/src
LIBS = -lz -llzo2 -L$(shell root-config --libdir) -lCore -lpthread -Lzopfli -lzopfli
CXX=$(shell root-config --cc)
SOFLAGS = -shared -g -Bdynamic

all: libLzoRoot.so

clean:
	rm -f *~ libLzoRoot.so *.o lz4/*.o

libLzoRoot.so: libZipRoot.o libLzoRoot.o lz4/lz4.o zopfli/libzopfli.so
	$(CXX) -o $@ $(SOFLAGS) $^ $(LIBS)
libLzoRoot.o: libLzoRoot.c lz4/lz4.c

zopfli/libzopfli.so: zopfli

zopfli:
	$(MAKE) -C zopfli libzopfli

.PHONY: zopfli
