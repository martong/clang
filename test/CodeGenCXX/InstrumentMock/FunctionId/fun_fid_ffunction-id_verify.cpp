// Test __function_id
// RUN: %clang_cc1 -O0 -disable-llvm-optzns -std=c++14 -triple=x86_64-apple-macosx10.11.0 -emit-llvm -verify -o - %s
void foo();

int main(){
    auto q = __function_id foo; // expected-error {{invalid use of '__function_id' intrinsic}}
}
