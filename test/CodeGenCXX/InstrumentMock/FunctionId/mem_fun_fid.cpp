// Test __function_id
// RUN: %clang_cc1 -O0 -disable-llvm-optzns -std=c++14 -triple=x86_64-apple-macosx10.11.0 -emit-llvm -o - %s | FileCheck %s
struct X {
    virtual int xxx(int a);
};

int main(){
    // CHECK: store i8* bitcast (i32 (i32)* @_ZN1X3xxxEi to i8*), i8** %y,
    void* y = (void*)__function_id X::xxx;
}
