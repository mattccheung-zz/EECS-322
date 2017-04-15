CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
CC_FLAGS := --std=c++11 -I./src -I./lib/PEGTL -g3
LD_FLAGS := 

all: dirs L2

dirs:
	mkdir -p obj ; mkdir -p bin ;

L2: $(OBJ_FILES)
	g++ $(LD_FLAGS) -o ./bin/$@ $^

obj/%.o: src/%.cpp
	g++ $(CC_FLAGS) -c -o $@ $<

test: L2
	./scripts/test.sh

clean:
	rm -f bin/L2 obj/* *.out *.o *.S core.* tests/liveness/*.tmp
