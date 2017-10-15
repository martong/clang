// Test __function_id
// RUN: %clang_cc1 -O0 -disable-llvm-optzns -std=c++14 -triple=x86_64-apple-macosx10.11.0 -emit-llvm -o - %s | FileCheck %s
template <typename T>
void foo();

int main(){
    // CHECK: store void ()* @_Z3fooIiEvv, void ()** %q
    auto q = __function_id foo<int>;
}
