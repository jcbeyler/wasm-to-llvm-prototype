# Wasm To LLVM Prototype

This repository implements a prototypical Wasm (in S-Expression form) to the LLVM IR. As with the Spec implementation, it is written for clarity and simplicity, _not_ speed (although it should be reasonably fast). Hopefully, it can be useful as a playground for trying out ideas and a device for nailing down performance related questions. In no way, is this supposed to be an alternative to the spec implementation.

The Spec implementation is the reference for semantical questions. Here, we are conforming to the spec definitions and will update the parser as the spec changes. For that reason, the code uses the spec uni-tests verbatim.

Currently, it can

* *parse* a few of the spec test cases
* *generate* LLVM IR and dumps it for each module file
* *generate* the assertion opcodes to validate the LLVM IR
* *generate* a single *execute_asserts* method that calls each assert, allowing easy testing with a very simple driver

The prototype currently does the following:

* Uses Flex/Bison to do the original parsing, this is most likely temporary but was easy to set up
* Parses the module, exports, and assertions
* Create an internal IR that represents Wasm nodes
* Generates the LLVM IR
* Runs the LLVM O2 Optimizations on the functions
* Generates the assertion code and one single assertion driver

The prototype does not do:

* Assert traps are not yet supported (I tried but got side-tracked, it's a WIP)
** It currently just signals that traps are not supported...
* Probably a lot more things

Things that I know we need to improve:

* The makefile; really did a hack job there
* The flex/bison analyzers will probably need to get ripped out and we could put something else if we want to keep the IR -> LLVM IR
* The coding convention probably
* The uni-testing of the code itself, I've used asserts to ensure that I find the todos left behind my trail blazing
* There are TODOs in the code that need to be handled

Participation is welcome. Discussions about new features, significant semantic
changes, or any specification change likely to generate substantial discussion
should take place in
[the WebAssembly design repository](https://github.com/WebAssembly/design)
first, so that this wasm-to-llvm-prototype repository can remain focused. And please follow the
[guidelines for contributing](Contributing.md).

## Building

This uses:

* Flex 2.5.35
* Bison 2.5
* LLVM 3.7

Once you have those, you should be able to do:

```
make
```

You'll get an executable named llvm_wasm.

## Testsuite Submodule

The test framework is built on having the .wast files there but in order to make things simpler, I have gone down the submodule route. So now, you must initalize and update it.

```
git submodule init
git submodule update
```

## Synopsis

You can call the executable with:

```
llvm_wasm <wasm filename>
```

where `wasm filename` is a script file (see below) to be run. See the spec test files and project for the definition of the syntax. This projects conforms to the format there and should not have discrepancies for long. Of course, as the spec repo moves differently, there might be times where the files there do not conform with this project. As times goes forward, I expect that to slow down and should no longer happen as often.

## Language

For most part, the language understood by the compiler is the one from the spec. There still are some todos to get it up to par but the goal is not to diverge from there. There might be some tests being done to afterwards influence the spec language but the core should remain spec-compliant.

## Implementation

The implementation consists of the following folders:

* *src*: the core code, see below

* *obj*: the built objects

* tests*: test folder with copies of the spec test files that are supported, meaning that the compiler does generate llvm code for them.

The code itself is divided in major components:

* *opt.flex* : the Flex lexer that reads the input code

* *opt.ypp* : the Bison parser that creates the AST

* *debug.h* : changes debug information being printed by the lexer and parser

* *wasm_file.h* : entry point of code generation since it represents the whole file

* *other.cpp/.h* : all the rest of the files :)

## What Next?

* TODOs: clean code, full wasm S-expression support, assertion trap test compliance, wrapper around for performance analysis
