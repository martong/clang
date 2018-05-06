// RUN: %clang_cc1 -emit-pch -o %t.1.ast %S/Inputs/fake_bar.c
// RUN: %clang_cc1 -emit-pch -o %t.2.ast %S/Inputs/foo.c
// RUN: %clang_cc1 -emit-pch -o %t.3.ast %S/Inputs/test.c

//// Dump the merged ASTs
// RUN: %clang_cc1 -ast-merge %t.1.ast -ast-merge %t.2.ast -ast-merge %t.3.ast -ast-dump -fcolor-diagnostics /dev/null

// %clang %S/test.c -###

//// Merge the ASTs and emit an object
// RUN: %clang_cc1 -ast-merge %t.1.ast -ast-merge %t.2.ast -ast-merge %t.3.ast /dev/null -emit-obj -o %t.o
//// Link
// RUN: %clang -o %t.output %t.o

// RUN: %t.output
