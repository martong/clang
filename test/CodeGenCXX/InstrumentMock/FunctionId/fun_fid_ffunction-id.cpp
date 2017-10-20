// Test __function_id
// RUN: %clang_cc1 -O0 -disable-llvm-optzns -std=c++14 -triple=x86_64-apple-macosx10.11.0 -emit-llvm -ffunction-id -o - %s | FileCheck %s
void foo();

int main(){
    // CHECK: store void ()* @_Z3foov, void ()** %q
    auto q = __function_id foo;
}
