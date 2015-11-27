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

CWJOBS := $(foreach ALG,$(ALGS),$(addprefix callgrind.${ALG}.,${LEVELS}))
MWJOBS := $(foreach ALG,$(ALGS),$(addprefix massif.${ALG}.,${LEVELS}))

all: callgrind-write massif-write
callgrind-write: ${CWJOBS}
massif-write: ${MWJOBS}

define CWJOBTEMPLATE
callgrind.$(1).$(2): todevnull
  valgrind --callgrind-out-file=$$@ --tool=callgrind -v --dump-instr=yes --trace-jump=yes --smc-check=all-non-file ./todevnull $(1) $(2)
endef
define MWJOBTEMPLATE
massif.$(1).$(2): todevnull
	valgrind --massif-out-file=$$@ --tool=massif --peak-inaccuracy=0.5 ./todevnull $(1) $(2)
endef
$(foreach ALG,$(ALGS),$(foreach LVL,${NUMBERS},$(eval $(call CWJOBTEMPLATE,$(ALG),$(LVL)))))
$(foreach ALG,$(ALGS),$(foreach LVL,${NUMBERS},$(eval $(call MWJOBTEMPLATE,$(ALG),$(LVL)))))



.PHONY: ${CWJOBS}

