// Test __function_id
// RUN: %clang_cc1 -std=c++14 -verify %s
struct X {
    virtual int xxx(int a);
};

int main(){
    void* x = (void*)&X::xxx; // expected-error {{cannot cast from type}}
}
