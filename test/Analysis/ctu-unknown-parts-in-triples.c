// We do not expect any error when one part of the triple is unknown, but other
// known parts are equal.

// RUN: mkdir -p %T/ctudir3
// RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -emit-pch -o %T/ctudir3/ctu-other.c.ast %S/Inputs/ctu-other.c
// RUN: cp %S/Inputs/externalFnMap2.txt %T/ctudir3/externalFnMap.txt
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fsyntax-only -std=c89 -analyze -analyzer-checker=core,debug.ExprInspection  -analyzer-config experimental-enable-naive-ctu-analysis=true -analyzer-config ctu-dir=%T/ctudir3 -verify %s

// expected-no-diagnostics

int f(int);

int main() {
  return f(5);
}
