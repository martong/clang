// Test -fsanitize=mock
// RUN: %clang_cc1 -O0 -fsanitize=mock -disable-llvm-optzns -std=c++14 -triple=x86_64-apple-macosx10.11.0 -emit-llvm -o - %s | FileCheck %s

struct X {
  void foo(int) {}
};

void MemberCaller(X& x) {
  x.foo(13);
}

// CHECK: ret
