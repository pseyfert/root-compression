OBJS = backward_references.o block_splitter.o brotli_bit_stream.o dictionary.o encode.o encode_parallel.o entropy_encode.o histogram.o literal_cost.o metablock.o static_dict.o streams.o utf8_util.o

%.o: %.cc %.h
	g++ -c $< -o $@ -fPIC -ggdb

libenc.so: $(OBJS)
	g++ -shared -o libenc.so $^ -ggdb

all : libenc.so

clean :
	rm -f $(OBJS) libenc.so

