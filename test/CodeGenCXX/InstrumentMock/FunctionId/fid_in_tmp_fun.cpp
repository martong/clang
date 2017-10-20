// Test __function_id
// RUN: %clang_cc1 -O0 -disable-llvm-optzns -std=c++14 -triple=x86_64-apple-macosx10.11.0 -emit-llvm -fsanitize=mock -o - %s | FileCheck %s
template <typename T>
void foo(T t) {}

template <typename T>
void bar() {
    // CHECK: store void (double)* @_Z3fooIdEvT_, void (double)** %p
    auto p = __function_id foo<T>;
}

template void bar<double>();
