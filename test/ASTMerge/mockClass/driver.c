// RUN: %clang_cc1 -x c++ -emit-pch -o %t.1.ast %S/Inputs/mock_bar.h
// RUN: %clang_cc1 -x c++ -emit-pch -o %t.2.ast %S/Inputs/foo.h
// RUN: %clang_cc1 -x c++ -emit-pch -o %t.3.ast %S/Inputs/test.c

//// Dump the merged ASTs
// RUN: %clang_cc1 -ast-merge %t.1.ast -ast-merge %t.2.ast -ast-merge %t.3.ast -ast-dump -fcolor-diagnostics /dev/null

// %clang %S/test.c -###

//// Merge the ASTs and emit an object
// RUN: %clang_cc1 -ast-merge %t.1.ast -ast-merge %t.2.ast -ast-merge %t.3.ast /dev/null -x c++ -emit-obj -o %t.o
//// Link
// RUN: %clang -o %t.output %t.o

// RUN: %t.output
