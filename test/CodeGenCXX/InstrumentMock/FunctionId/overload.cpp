// Test __function_id
// RUN: %clang_cc1 -O0 -disable-llvm-optzns -std=c++14 -triple=x86_64-apple-macosx10.11.0 -emit-llvm -o - %s | FileCheck %s
int foo(int);
int foo(double);

struct Foo {
    int mf(int) { return 3; }
    int mf(double) { return 4; }
};

void bar(){
    // CHECK: store i32 (double)* @_Z3food, i32 (double)** %pf
    int (*pf)(double) = __function_id foo;

    // CHECK: store { i64, i64 } { i64 ptrtoint (i32 (%struct.Foo*, i32)* @_ZN3Foo2mfEi to i64), i64 0 }, { i64, i64 }* %mpf
    int (Foo::*mpf)(int) = &Foo::mf; // selects int mf(int)

    //int (*mpf2)(int) = __function_id Foo::mf; // selects int mf(int)
}
