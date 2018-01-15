// RUN: mkdir -p %T/xtudir3
// RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -emit-pch -o %T/xtudir3/xtu-other.c.ast %S/Inputs/xtu-other.c
// RUN: cp %S/Inputs/externalFnMap2_usr.txt %T/xtudir3/externalFnMap.txt
// RUN: %clang_cc1 -triple powerpc64-montavista-linux-gnu -fsyntax-only -std=c89 -analyze -analyzer-checker=core,debug.ExprInspection -analyzer-config xtu-dir=%T/xtudir3 -verify %s

// We expect an error in this file, but without a location.
// expected-error-re@./xtu-different-triples.c:*{{imported AST from {{.*}} had been generated for a different target}}

int f(int);

int main() {
  return f(5); // TODO expect the error here at the CallExpr location
}
