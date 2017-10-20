// Test __function_id
// RUN: %clang_cc1 -O0 -disable-llvm-optzns -std=c++14 -triple=x86_64-apple-macosx10.11.0 -emit-llvm -fsanitize=mock -o - %s | FileCheck %s
template <typename T>
int foo(T, int);

int foo(double, int);

void bar(){
    // CHECK: store i32 (i32, i32)* @_Z3fooIiEiT_i, i32 (i32, i32)** %pf
    int (*pf)(int, int) = __function_id foo<int>;
}
