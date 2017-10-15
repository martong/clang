// Test __function_id
// RUN: %clang_cc1 -std=c++14 -verify %s
// expected-no-diagnostics
struct X {
    X();
};

int main(){
    //void* y = (void*)__function_id X::X;
}
