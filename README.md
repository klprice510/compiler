# compiler
Compiler for modified version of C++ into x86_64 assemble.

This is C code to create a compiler for a modified C++ language in the following steps:
  1.  Uses an open source lexical analyzer, Flex, to identify all accepted "words" (lexer.l).
  2.  Uses an open source parser, Bison, to create a syntax tree (parser.y).
  3.  Semantic checking, symblol tables building, variable declartions checking, type checking, etc (120++.c)
  4.  Three address intermediate code created (120++.c)
  5.  Assemble code created for x86_64
  
  This project was done for a computer science class at the University of Idaho, CS 445 Complier Design
