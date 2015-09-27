# root-compression

This is partially a project to educate myself how things work.
Beyond the educational aspect, the project is to add new compression algorithms
to ROOT. It is based on previous work by Manuel Schiller, adding LZO and LZ4 to
ROOT.

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
```

# how to run

Override the root default compression stuff by injecting the library into `LD_PRELOAD`:

```
cd path/to/root-compression
export LD_PRELOAD=`pwd`/zopfli/libzopfli.so:`pwd`/libLzoRoot.so
```

test your setup by putting a root file with a TTree in it (at top level) as
`org.root` into the `test` subdirectory and run
```
cd path/to/root-compression/test
make
```
now you should have many many root files with your original TTree in various compressions.

## about zopfli

I use zopfli from `git@github.com:pseyfert/zopfli.git` and the branch
`myadditions`.  This is a fork from `https://github.com/google/zopfli.git` at
commit `89cf773beef75d7f4d6d378debdf299378c3314e`.
