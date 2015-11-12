
# A bit complicated of a makefile done by hand, I'll replace it by an automatic version soon...
# It does what needs to be done for now...

EXE = llvm_wasm
HEADERS = $(wildcard src/*h) $(wildcard src/parser/*h) $(wildcard src/parser/*h)
GENERATED = obj/lex.yy.c obj/wasm.tab.cpp
GENERATED_OBJ = obj/lex.yy.o obj/wasm.tab.o
SRC_OBJ = $(patsubst src/%.cpp, obj/%.o, $(wildcard src/*.cpp))
PARSER_SRC_OBJ = $(patsubst src/parser/%.cpp, obj/%.o, $(wildcard src/parser/*.cpp))
PASSES_SRC_OBJ = $(patsubst src/passes/%.cpp, obj/%.o, $(wildcard src/passes/*.cpp))

FILES = ${HEADERS} ${SRC_OBJ} ${GENERATED}
OBJS = $(SRC_OBJ) $(PARSER_SRC_OBJ) $(GENERATED_OBJ) $(PASSES_SRC_OBJ)

INCLUDEDIR = -I`llvm-config --includedir` -Isrc/parser -Isrc -Isrc/passes
CFLAGS = -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -g -std=gnu++0x $(INCLUDEDIR) -O3
LIBDIR = `llvm-config --libdir`
LIBS = -ly -lfl -L$(LIBDIR) -lLLVM

$(EXE): ${OBJS} $(HEADERS)
	g++ -o $@ ${CFLAGS} ${OBJS} ${LIBS}

obj/lex.yy.c: src/parser/wasm.flex obj/wasm.tab.hpp $(HEADERS)
	flex -o $@ $<

obj/lex.yy.o: obj/lex.yy.c
	g++ -c -o $@ $< ${CFLAGS}

obj/wasm.tab.o: obj/wasm.tab.cpp obj/wasm.tab.hpp
	g++ -c -o $@ $< ${CFLAGS}

obj/wasm.tab.hpp: src/parser/wasm.ypp $(HEADERS)
	bison --defines $< -o obj/wasm.tab.cpp

obj/wasm.tab.cpp: src/parser/wasm.ypp $(HEADERS)
	bison --defines $< -o $@

$(SRC_OBJ):obj/%.o: src/%.cpp $(HEADERS)
	g++ -c -o $@ $< ${CFLAGS}

$(PARSER_SRC_OBJ):obj/%.o: src/parser/%.cpp $(HEADERS)
	g++ -c -o $@ $< ${CFLAGS}

$(PASSES_SRC_OBJ):obj/%.o: src/passes/%.cpp $(HEADERS)
	g++ -c -o $@ $< ${CFLAGS}

perf-test: $(EXE)
	perf_tests/run.sh

test: $(EXE)
	wrapper/run.sh

update-modules:
	git submodule foreach git pull origin master

clean:
	rm -f $(EXE) obj/*

