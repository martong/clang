// RUN: %clang++ -x c++-header -std=c++11 -o %t.1.ast %S/Inputs/il.cpp
// RUN: %clang_cc1 -x c++ -std=c++11 -ast-merge %t.1.ast -fsyntax-only %s 2>&1 | FileCheck --allow-empty %s
// CHECK-NOT: unsupported AST node
