%{
/*
// Copyright (c) 2015 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#include "debug.h"
#include "enums.h"
#include "wasm.tab.hpp"
#include "globals.h"
#include "utility.h"

#include <cassert>

#define LEX_DEBUG_PRINT(...) \
    DEBUG_PRINT(LEX_GROUP, LEX_VERBOSITY, __VA_ARGS__)

%}

DIGIT    [0-9]
HEX_DIGIT    [0-9a-fA-F]
ID       [a-z_A-Z][\.\-A-Za-z0-9_]*

%%

lt {
  LEX_DEBUG_PRINT("LT\n");
  yylval.l = LT_OPER;
  return LT;
}

le {
  LEX_DEBUG_PRINT("LE\n");
  yylval.l = LE_OPER;
  return LE;
}

gt {
  LEX_DEBUG_PRINT("GT\n");
  yylval.l = GT_OPER;
  return GT;
}

ge {
  LEX_DEBUG_PRINT("GE\n");
  yylval.l = GE_OPER;
  return GE;
}

ne {
  LEX_DEBUG_PRINT("NE\n");
  yylval.l = NE_OPER;
  return NE;
}

eq {
  LEX_DEBUG_PRINT("EQ\n");
  yylval.l = EQ_OPER;
  return EQ;
}

add {
  LEX_DEBUG_PRINT("ADD\n");
  yylval.l = ADD_OPER;
  return ADD;
}

sub {
  LEX_DEBUG_PRINT("SUB\n");
  yylval.l = SUB_OPER;
  return SUB;
}

const {
  LEX_DEBUG_PRINT("CONST\n");
  return CONST;
}

mul {
  LEX_DEBUG_PRINT("MUL\n");
  yylval.l = MUL_OPER;
  return MUL;
}

div {
  LEX_DEBUG_PRINT("DIV\n");
  yylval.l = DIV_OPER;
  return DIV;
}

rem {
  LEX_DEBUG_PRINT("REM\n");
  yylval.l = REM_OPER;
  return REM;
}

and {
  LEX_DEBUG_PRINT("AND\n");
  yylval.l = AND_OPER;
  return AND;
}

or {
  LEX_DEBUG_PRINT("OR\n");
  yylval.l = OR_OPER;
  return OR;
}

xor {
  LEX_DEBUG_PRINT("XOR\n");
  yylval.l = XOR_OPER;
  return XOR;
}

shl {
  LEX_DEBUG_PRINT("SHL\n");
  yylval.l = SHL_OPER;
  return SHL;
}

shr {
  LEX_DEBUG_PRINT("SHR\n");
  yylval.l = SHR_OPER;
  return SHR;
}

clz {
  LEX_DEBUG_PRINT("CLZ\n");
  yylval.l = CLZ_OPER;
  return CLZ;
}

ctz {
  LEX_DEBUG_PRINT("CTZ\n");
  yylval.l = CTZ_OPER;
  return CTZ;
}

popcnt {
  LEX_DEBUG_PRINT("POPCNT\n");
  yylval.l = POPCNT_OPER;
  return POPCNT;
}

sqrt {
  LEX_DEBUG_PRINT("SQRT\n");
  yylval.l = SQRT_OPER;
  return SQRT;
}

max {
  LEX_DEBUG_PRINT("MAX\n");
  yylval.l = MAX_OPER;
  return MAX;
}

memory {
  LEX_DEBUG_PRINT("MEMORY\n");
  return MEMORY;
}

min {
  LEX_DEBUG_PRINT("MIN\n");
  yylval.l = MIN_OPER;
  return MIN;
}

ceil {
  LEX_DEBUG_PRINT("CEIL\n");
  yylval.l = CEIL_OPER;
  return CEIL;
}

floor {
  LEX_DEBUG_PRINT("FLOOR\n");
  yylval.l = FLOOR_OPER;
  return FLOOR;
}

trunc {
  LEX_DEBUG_PRINT("TRUNC\n");
  yylval.l = TRUNC_OPER;
  return TRUNC;
}

nearest {
  LEX_DEBUG_PRINT("NEAREST\n");
  yylval.l = NEAREST_OPER;
  return NEAREST;
}

abs {
  LEX_DEBUG_PRINT("ABS\n");
  yylval.l = ABS_OPER;
  return ABS;
}

neg {
  LEX_DEBUG_PRINT("NEG\n");
  yylval.l = NEG_OPER;
  return NEG;
}

copysign {
  LEX_DEBUG_PRINT("COPYSIGN\n");
  yylval.l = COPYSIGN_OPER;
  return COPYSIGN;
}

nop {
  LEX_DEBUG_PRINT("NOP\n");
  return NOP;
}

reinterpret {
  LEX_DEBUG_PRINT("REINTERPRET\n");
  yylval.l = REINTERPRET_OPER;
  return REINTERPRET;
}

convert {
  LEX_DEBUG_PRINT("CONVERT\n");
  yylval.l = CONVERT_OPER;
  return CONVERT;
}

demote {
  LEX_DEBUG_PRINT("DEMOTE\n");
  yylval.l = DEMOTE_OPER;
  return DEMOTE;
}

promote {
  LEX_DEBUG_PRINT("PROMOTE\n");
  yylval.l = PROMOTE_OPER;
  return PROMOTE;
}

wrap {
  LEX_DEBUG_PRINT("WRAP\n");
  yylval.l = WRAP_OPER;
  return WRAP;
}

extend {
  LEX_DEBUG_PRINT("EXTEND\n");
  yylval.l = EXTEND_OPER;
  return EXTEND;
}

block {
  LEX_DEBUG_PRINT("BLOCK\n");
  return BLOCK_TOKEN;
}

if {
  LEX_DEBUG_PRINT("IF\n");
  return IF;
}

if_else {
  LEX_DEBUG_PRINT("IF ELSE\n");
  return IF_ELSE;
}

loop {
  LEX_DEBUG_PRINT("LOOP\n");
  return LOOP;
}

export {
  LEX_DEBUG_PRINT("EXPORT\n");
  return EXPORT_TOKEN;
}

br {
  LEX_DEBUG_PRINT("BREAK\n");
  return BREAK_TOKEN;
}

return {
  LEX_DEBUG_PRINT("RETURN\n");
  return RETURN_TOKEN;
}

result {
  LEX_DEBUG_PRINT("RESULT\n");
  return RESULT_TOKEN;
}

get_local {
  LEX_DEBUG_PRINT("GET LOCAL\n");
  return GET_LOCAL;
}

local {
  LEX_DEBUG_PRINT("LOCAL\n");
  return LOCAL_TOKEN;
}

label {
  LEX_DEBUG_PRINT("LABEL\n");
  return LABEL;
}

set_local {
  LEX_DEBUG_PRINT("SET LOCAL\n");
  return SET_LOCAL;
}

call {
  LEX_DEBUG_PRINT("CALL");
  return CALL_TOKEN;
}

assert_return {
  LEX_DEBUG_PRINT("ASSERT RETURN\n");
  return ASSERT_RETURN_TOKEN;
}

assert_return_nan {
  LEX_DEBUG_PRINT("ASSERT RETURN NAN\n");
  return ASSERT_RETURN_NAN_TOKEN;
}

assert_trap {
  LEX_DEBUG_PRINT("ASSERT TRAP\n");
  return ASSERT_TRAP_TOKEN;
}

assert_invalid {
  LEX_DEBUG_PRINT("ASSERT INVALID\n");
  return ASSERT_INVALID_TOKEN;
}

invoke {
  LEX_DEBUG_PRINT("INVOKE\n");
  return INVOKE_TOKEN;
}

module {
  LEX_DEBUG_PRINT("MODULE\n");
  return MODULE_TOKEN;
}

%{
/* Skipping switch, call, destruct */

// Skipping load/store

// Skipping const, unop, binop, relop, cvtop

// Skipping case
%}

func {
  LEX_DEBUG_PRINT("FUNC\n");
  return FUNCTION_TOKEN;
}

param {
  LEX_DEBUG_PRINT("PARAM\n");
  return PARAM_TOKEN;
}

store {
  LEX_DEBUG_PRINT("STORE\n");
  yylval.l = STORE_OPER;
  return STORE;
}

load {
  LEX_DEBUG_PRINT("LOAD\n");
  yylval.l = LOAD_OPER;
  return LOAD;
}

f32 {
  LEX_DEBUG_PRINT("Type %s\n", yytext);
  yylval.l = FLOAT_32;
  return TYPE;
}

f64 {
  LEX_DEBUG_PRINT("Type %s\n", yytext);
  yylval.l = FLOAT_64;
  return TYPE;
}

i32 {
  LEX_DEBUG_PRINT("Type %s\n", yytext);
  yylval.l = INT_32;
  return TYPE;
}

i64 {
  LEX_DEBUG_PRINT("Type %s\n", yytext);
  yylval.l = INT_64;
  return TYPE;
}

[-+]{0,1}0x{HEX_DIGIT}+ {
  LEX_DEBUG_PRINT("Integer %s\n", yytext);
  yylval.l = strtoull(yytext, nullptr, 16);
  return INTEGER;
}

[-+]{0,1}0x[012][\.]{0,1}{HEX_DIGIT}*p[-+]{0,1}{DIGIT}+ {
  LEX_DEBUG_PRINT("Hexa float %s\n", yytext);
  yylval.string = strdup(yytext);
  return FLOAT;
}

[-+]{0,1}infinity {
  LEX_DEBUG_PRINT("Infinity: %s\n", yytext);
  yylval.string = strdup(yytext);
  return FLOAT;
}

[-+]{0,1}nan:0x{HEX_DIGIT}+ {
  // Recreate the one that strtof and strtod prefer.

  // +2 because we want a ')' too.
  char* new_string = new char[strlen(yytext) + 2];
  strcpy(new_string, yytext);

  char* colon = strchr(new_string, ':');
  assert(colon != nullptr);
  *colon = '(';

  strcat(new_string, ")");

  yylval.string = new_string;
  return FLOAT;
}

[-+]{0,1}nan {
  LEX_DEBUG_PRINT("Nan: %s\n", yytext);
  yylval.string = strdup(yytext);
  return FLOAT;
}

[-+]{0,1}{DIGIT}+ {
  LEX_DEBUG_PRINT("Integer %s\n", yytext);
  char* end = nullptr;
  yylval.l = strtoll(yytext, &end, 0);
  assert(end != nullptr && *end == '\0');
  return INTEGER;
}

\"([^\"]|(\\\"))*\" {
  LEX_DEBUG_PRINT("String %s\n", yytext);
  int len = strlen(yytext);
  yytext[len - 1] = '\0';
  yylval.string = strdup(yytext + 1);
  return STRING;
}

[-+]{0,1}{DIGIT}{DIGIT}*"."{DIGIT}*[e]{0,1}[-+]{0,1}{DIGIT}+ {
  LEX_DEBUG_PRINT("Float %s\n", yytext);
  yylval.string = strdup(yytext);
  return FLOAT;
}

[-+]{0,1}{DIGIT}{DIGIT}*[e]{0,1}[-+]{0,1}{DIGIT}+ {
  LEX_DEBUG_PRINT("Float %s\n", yytext);
  yylval.string = strdup(yytext);
  return FLOAT;
}

[;]{2}.* {
}

[(][;].*[;][)] {
}

${ID} {
  LEX_DEBUG_PRINT("ID %s\n", yytext);
  yylval.string = strdup(yytext);
  return IDENTIFIER;
}

\n {
  LEX_DEBUG_PRINT("Handled line %d\n", Globals::Get()->GetLineCnt());
  Globals::Get()->IncrementLineCnt();
}

" " {
}

. {
  LEX_DEBUG_PRINT("Stray character %s\n", yytext);
  return yytext[0];
}
