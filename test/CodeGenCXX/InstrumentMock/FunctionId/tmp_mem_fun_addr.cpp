// Test __function_id
// RUN: %clang_cc1 -O0 -disable-llvm-optzns -std=c++14 -triple=x86_64-apple-macosx10.11.0 -emit-llvm -o - %s | FileCheck %s
struct X {
    template <typename T>
    int xxx(int a);
};

int main(){
    // CHECK: store { i64, i64 } { i64 ptrtoint (i32 (%struct.X*, i32)* @_ZN1X3xxxIiEEii to i64), i64 0 }, { i64, i64 }* %x
    auto x = & X::xxx<int>;
}
