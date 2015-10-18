OBJS = bit_reader.o decode.o dictionary.o huffman.o state.o streams.o

%.o: %.c %.h
	g++ -c $< -o $@ -fPIC -ggdb

libdec.so: $(OBJS)
	g++ -shared -o libdec.so $^ -ggdb

all : libdec.so

clean :
	rm -f $(OBJS) libdec.so

