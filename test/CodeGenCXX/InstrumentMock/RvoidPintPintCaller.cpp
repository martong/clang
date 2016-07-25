// Test -fsanitize=mock
// RUN: %clang_cc1 -O0 -fsanitize=mock -disable-llvm-optzns -std=c++14 -triple=x86_64-apple-macosx10.11.0 -emit-llvm -o - %s | FileCheck %s

void RvoidPintPint(int,int);
void RvoidPintPint_2(int, int);

void RvoidPintPintCaller() {
  RvoidPintPint(13, 13);
  RvoidPintPint_2(15, 15);
}

// CHECK: ret
