// Test __function_id
// RUN: %clang_cc1 -O0 -disable-llvm-optzns -std=c++14 -triple=x86_64-apple-macosx10.11.0 -emit-llvm -o - %s | FileCheck %s
int foo(int);
int foo(double);

void bar(){
    // CHECK: store i32 (double)* @_Z3food, i32 (double)** %pf
    int (*pf)(double) = __function_id foo;
}
