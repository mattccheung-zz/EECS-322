CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
CC_FLAGS  := --std=c++11 -I./src -I../lib/PEGTL -g3
LD_FLAGS  := 
CC        := g++

all: dirs L2

dirs:
	mkdir -p obj ; mkdir -p bin ;

L2: $(OBJ_FILES)
	$(CC) $(LD_FLAGS) -o ./bin/$@ $^

obj/%.o: src/%.cpp
	$(CC) $(CC_FLAGS) -c -o $@ $<

oracle: L2
	./scripts/generateOutput.sh

oracle_liveness: L2
	./scripts/generateOutputLiveness.sh

test: L2
	./scripts/test.sh

test_liveness: L2
	./scripts/testLiveness.sh

clean:
	rm -f bin/L2 obj/* *.out *.L1 *.o *.S core.* tests/liveness/*.tmp tests/*.tmp
