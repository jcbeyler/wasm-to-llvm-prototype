#include <iostream>

#include "debug.h"
#include "globals.h"

int yyparse();

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <filename> " << std::endl;
    return EXIT_FAILURE;
  }

  // Set up global variable singleton.
  Globals::Get()->SetFileName(argv[1]);

  BISON_PRINT("Parsing %s\n", argv[1]);

  freopen(argv[1], "r", stdin);
  yyparse();

  BISON_PRINT("Done Parsing %s\n", argv[1]);
  return EXIT_SUCCESS;
}
