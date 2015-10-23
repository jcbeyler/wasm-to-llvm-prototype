
# A bit complicated of a makefile done by hand, I'll replace it by an automatic version soon...
# It does what needs to be done for now...

EXE = llvm_wasm
HEADERS = $(wildcard src/*h)
GENERATED = obj/lex.yy.c obj/opt.tab.cpp
GENERATED_OBJ = obj/lex.yy.o obj/opt.tab.o
SRC_OBJ = $(patsubst src/%.cpp, obj/%.o, $(wildcard src/*.cpp))

FILES = ${HEADERS} ${SRC_OBJ} ${GENERATED}
OBJS = $(SRC_OBJ) $(GENERATED_OBJ)

CFLAGS = -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -g -std=gnu++0x -Isrc
LIBS = -ly -lfl -L/usr/lib/llvm-3.7/lib -lLLVM-3.7.0

$(EXE): ${OBJS} $(HEADERS)
	g++ -o $@ ${CFLAGS} ${OBJS} ${LIBS}

obj/lex.yy.c: src/opt.flex obj/opt.tab.hpp $(HEADERS)
	flex -o $@ $<

obj/lex.yy.o: obj/lex.yy.c
	g++ -c -o $@ $< ${CFLAGS}

obj/opt.tab.o: obj/opt.tab.cpp obj/opt.tab.hpp
	g++ -c -o $@ $< ${CFLAGS}

obj/opt.tab.hpp: src/opt.ypp $(HEADERS)
	bison --defines $< -o obj/opt.tab.cpp

obj/opt.tab.cpp: src/opt.ypp $(HEADERS)
	bison --defines $< -o $@

$(SRC_OBJ):obj/%.o: src/%.cpp $(HEADERS)
	g++ -c -o $@ $< ${CFLAGS}

test: $(EXE)
	wrapper/run.sh

clean:
	rm $(EXE) obj/*

