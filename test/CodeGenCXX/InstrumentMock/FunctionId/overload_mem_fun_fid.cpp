// Test __function_id
// RUN: %clang_cc1 -O0 -disable-llvm-optzns -std=c++14 -triple=x86_64-apple-macosx10.11.0 -emit-llvm -fsanitize=mock -o - %s | FileCheck %s
struct Foo {
    int mf(int) { return 3; }
    int mf(double) { return 4; }
};

void bar(){
    // CHECK: store i32 (i32)* bitcast (i32 (%struct.Foo*, i32)* @_ZN3Foo2mfEi to i32 (i32)*), i32 (i32)** %mpf
    int (*mpf)(int) = __function_id Foo::mf; // selects int mf(int)
}
