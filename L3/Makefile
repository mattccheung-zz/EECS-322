CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
CC_FLAGS  := --std=c++11 -I./src -I../lib/PEGTL -g3
LD_FLAGS  := 
CC        := g++

all: dirs L3

dirs:
	mkdir -p obj ; mkdir -p bin ;

L3: $(OBJ_FILES)
	$(CC) $(LD_FLAGS) -o ./bin/$@ $^

obj/%.o: src/%.cpp
	$(CC) $(CC_FLAGS) -c -o $@ $<

clean:
	rm -f bin/L3 obj/* *.out *.L1 *.o *.S tests/*.tmp
