ifndef FF_ROOT 
FF_ROOT = ${HOME}/fastflow
endif


CXX       = g++ -std=c++17 
INCLUDES  = -I $(FF_ROOT) -I ./genetic
CXXFLAGS  = -g -DBLOCKING_MODE -DFF_BOUNDED_BUFFER

LDFLAGS   = -pthread
OPTFLAGS  = -O3 # -finline-functions -DNDEBUG

TARGETS   = gen_tsp_seq \
            gen_tsp_par \
            gen_tsp_ff \

.PHONY: all clean cleanall
.SUFFIXES: .cpp 


%: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPTFLAGS) -o build/$@ $< $(LDFLAGS)

all: build/$(TARGETS)

clean:
	rm -f build/$(TARGETS)

cleanall: clean
	rm -f *.o *~
