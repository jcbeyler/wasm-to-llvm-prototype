// Allow libfl.so to access the yylex() function. libfl.so expects yylex() to be
// compiled for C, but we compiled for C++. In this file we define a C function
// yylex() (that do not conflict with the C++ function of the same name). This
// calls another function yylex_cpp() that will effectively call the C++ version
// of yylex().

extern "C" {
  int yylex();
}

int yylex_cpp();

int yylex() {
  return yylex_cpp();
}
