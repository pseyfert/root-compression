#ROOT_ENV: aliased to export CPPFLAGS="`root-config --cflags` -march=native -g -Wextra -Wall -Wshadow" ; export LDFLAGS="`root-config --libs` -lRooStats -lRooFitCore -lRooFit -lMinuit -lFoam -lMathMore -lTMVA -lstdc++ -m64 -g -march=native"; export CPP=c++; export CXX=c++; export CC=c++

parent.h: org.root
	root -l -b -q generate.C

parent.C: org.root
	root -l -b -q generate.C

read: read.o parent.o
 
# http://stackoverflow.com/questions/1490949/how-to-write-loop-in-makefile
LAST_LEVEL := 9
LEVELS := $(shell seq 1 ${LAST_LEVEL})

ALGS := 1 2 4 5 6 7

CWJOBS   := $(foreach ALG,$(ALGS),$(addprefix $(addprefix callgrind-write.${ALG}.,${LEVELS}),.out))
MWJOBS   := $(foreach ALG,$(ALGS),$(addprefix $(addprefix massif-write.${ALG}.,${LEVELS}),   .out))
RWJOBS   := $(foreach ALG,$(ALGS),$(addprefix $(addprefix realtime-write.${ALG}.,${LEVELS}), .log))
SIZEJOBS := $(foreach ALG,$(ALGS),$(addprefix $(addprefix size.${ALG}.,${LEVELS}),           .root))

all: callgrind-write massif-write realtime-write size
callgrind-write: ${CWJOBS}
massif-write: ${MWJOBS}
realtime-write: ${RWJOBS}
size: ${SIZEJOBS}

define RWJOBTEMPLATE
realtime-write.$(1).$(2).log: writetime
	LD_PRELOAD=$(LD_PRELOAD):../brotli/enc/libenc.so:../brotli/dec/libdec.so:../zopfli/libzopfli.so:../libLzoRoot.so LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):../brotli/enc:../brotli/dec" ./writetime $(1) $(2) > $$@
endef
define CWJOBTEMPLATE
callgrind-write.$(1).$(2).out: todevnull
	LD_PRELOAD=$(LD_PRELOAD):../brotli/enc/libenc.so:../brotli/dec/libdec.so:../zopfli/libzopfli.so:../libLzoRoot.so LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):../brotli/enc:../brotli/dec" valgrind --callgrind-out-file=$$@ --tool=callgrind -v --dump-instr=yes --trace-jump=yes --smc-check=all-non-file ./todevnull $(1) $(2)
endef
define MWJOBTEMPLATE
massif-write.$(1).$(2).out: todevnull
	LD_PRELOAD=$(LD_PRELOAD):../brotli/enc/libenc.so:../brotli/dec/libdec.so:../zopfli/libzopfli.so:../libLzoRoot.so LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):../brotli/enc:../brotli/dec" valgrind --massif-out-file=$$@ --tool=massif --peak-inaccuracy=0.5 ./todevnull $(1) $(2)
endef
define SIZETEMPLATE
size.$(1).$(2).root: convert
	LD_PRELOAD=$(LD_PRELOAD):../brotli/enc/libenc.so:../brotli/dec/libdec.so:../zopfli/libzopfli.so:../libLzoRoot.so LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):../brotli/enc:../brotli/dec" ./convert $(1) $(2)
endef
$(foreach ALG,$(ALGS),$(foreach LVL,${LEVELS},$(eval $(call CWJOBTEMPLATE,$(ALG),$(LVL)))))
$(foreach ALG,$(ALGS),$(foreach LVL,${LEVELS},$(eval $(call MWJOBTEMPLATE,$(ALG),$(LVL)))))
$(foreach ALG,$(ALGS),$(foreach LVL,${LEVELS},$(eval $(call RWJOBTEMPLATE,$(ALG),$(LVL)))))
$(foreach ALG,$(ALGS),$(foreach LVL,${LEVELS},$(eval $(call SIZETEMPLATE,$(ALG),$(LVL)))))



.PHONY:

