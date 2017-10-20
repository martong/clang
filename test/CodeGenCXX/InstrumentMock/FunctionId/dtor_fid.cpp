// Test __function_id
// RUN: %clang_cc1 -O0 -disable-llvm-optzns -std=c++14 -triple=x86_64-apple-macosx10.11.0 -emit-llvm -fsanitize=mock -o - %s | FileCheck %s
struct X {
    ~X();
};

int main(){
    // CHECK: store void ()* @_ZN1XD2Ev, void ()** %y
    auto y = __function_id X::~X;
}
