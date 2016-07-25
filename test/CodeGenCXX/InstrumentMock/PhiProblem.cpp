// Test -fsanitize=mock
// RUN: %clang_cc1 -O0 -fsanitize=mock -disable-llvm-optzns -std=c++14 -triple=x86_64-apple-macosx10.11.0 -emit-llvm -o - %s | FileCheck %s

void RvoidPint_5(int);

static int square(int a) { return a*a; }

void PhiProblem(int a) {
    RvoidPint_5(square(a));
}

// CHECK: ret
