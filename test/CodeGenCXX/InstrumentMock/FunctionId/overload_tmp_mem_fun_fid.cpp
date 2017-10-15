// Test __function_id
// RUN: %clang_cc1 -O0 -disable-llvm-optzns -std=c++14 -triple=x86_64-apple-macosx10.11.0 -emit-llvm -o - %s | FileCheck %s
struct Foo {
    template <typename T>
    int mf(T, int);

    int mf(double, int);
};

void bar(){
    // CHECK: store i32 (i32, i32)* @_ZN3Foo2mfIiEEiT_i, i32 (i32, i32)** %mpf
    int (*mpf)(int, int) = __function_id Foo::mf<int>; // selects int mf(int, int)
}
