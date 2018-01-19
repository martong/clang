// We do not expect any error when one part of the triple is unknown, but other
// known parts are equal.

// RUN: mkdir -p %T/xtudir3
// RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -emit-pch -o %T/xtudir3/xtu-other.c.ast %S/Inputs/xtu-other.c
// RUN: cp %S/Inputs/externalFnMap2_usr.txt %T/xtudir3/externalFnMap.txt
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fsyntax-only -std=c89 -analyze -analyzer-checker=core,debug.ExprInspection -analyzer-config xtu-dir=%T/xtudir3 -verify %s

// expected-no-diagnostics

int f(int);

int main() {
  return f(5);
}
