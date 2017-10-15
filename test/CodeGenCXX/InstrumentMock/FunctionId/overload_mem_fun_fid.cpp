// Test __function_id
// RUN: %clang_cc1 -O0 -disable-llvm-optzns -std=c++14 -triple=x86_64-apple-macosx10.11.0 -emit-llvm -o - %s | FileCheck %s
struct Foo {
    int mf(int) { return 3; }
    int mf(double) { return 4; }
};

void bar(){
    // CHECK: store i8* bitcast
    int (*mpf)(int) = __function_id Foo::mf; // selects int mf(int)
}
