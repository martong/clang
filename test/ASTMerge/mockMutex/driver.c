// RUN: %clang++ -x c++-header -o %t.1.ast %S/Inputs/MockMutex.hpp
// RUN: %clang++ -x c++-header -o %t.2.ast %S/Inputs/Entity.hpp
// RUN: %clang++ -x c++-header -o %t.3.ast %S/Inputs/test.cpp

//// Merge the ASTs and emit an object
// RUN: %clang_cc1 -x c++ -ast-merge %t.1.ast -ast-merge %t.2.ast -ast-merge %t.3.ast %s -emit-obj -o %t.o
//// Link
// RUN: %clang++ -o %t.output %t.o

// RUN: %t.output
