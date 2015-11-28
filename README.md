# root-compression

This is partially a project to educate myself how things work.
Beyond the educational aspect, the project is to add new compression algorithms
to ROOT. It is based on previous work by Manuel Schiller, adding LZO and LZ4 to
ROOT.

# where to get the underlying algorithms

LZO should ship with your OS. otherwise: http://www.oberhumer.com/opensource/lzo/#download
LZ4 is included in the repo. originally: https://github.com/Cyan4973/lz4
zopfli is forked (see below). originally:https://github.com/google/zopfli.git
brotli is forkedly included. originally: https://github.com/google/brotli

# how to build

Get zopfli and put it `path/to/root-compression/zopfli`.
Call make from `path/to/root-compression`.
Observe that this implicitly calls
```
cd path/to/root-compression/zopfli
make libzopfli
```
You should now have to shared libraries:
```
path/to/root-compression/libLzoRoot.so
path/to/root-compression/zopfli/libzopfli.so
path/to/root-compression/brotli/enc/libenc.so
path/to/root-compression/brotli/dec/libdec.so
```

# how to run

Override the root default compression stuff by injecting the library into `LD_PRELOAD`:

```
cd path/to/root-compression
export LD_PRELOAD=$LD_PRELOAD:`pwd`/brotli/enc/libenc.so:`pwd`/brotli/dec/libdec.so:`pwd`/zopfli/libzopfli.so:`pwd`/libLzoRoot.so
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/brotli/enc:`pwd`/brotli/dec

```

test your setup by putting a root file with a TTree in it (at top level) as
`org.root` into the `test` subdirectory and run
```
cd path/to/root-compression/test
make
```
or
```
cd path/to/root-compression
make test
```
now you should have many many root files with your original TTree in various compressions.

## about zopfli

I use zopfli from `git@github.com:pseyfert/zopfli.git` and the branch
`myadditions`.  This is a fork from `https://github.com/google/zopfli.git` at
commit `89cf773beef75d7f4d6d378debdf299378c3314e`.

## running tests

facing the transition to automised tests (in the test-dev branch) I'd like to
point to http://stackoverflow.com/questions/1490949/how-to-write-loop-in-makefile
according to which one can specify the test range
```
make -C test size -j 10 LAST_LEVEL=4 # to only test compression levels 1..4
make -C test callgrind-write -j 20 ALGS="6 7" LEVELS="4 5" # test only zopfli and brotli, and only levels 4 and 5
```
Furthermore, one does not need to specify LD_PRELOAD and LD_LIBRARY_PATH by hand
anymore to run tests.

## results

at the moment at https://twiki.cern.ch/twiki/bin/view/Main/PaulSeyfert#root_compression

or in plot.C

cf
![plot](http://mathphys.fsk.uni-heidelberg.de/~pseyfert/compressionalgs_in_root.png)