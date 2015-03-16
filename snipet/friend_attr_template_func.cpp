#include <assert.h>
class A {
  int a = 0;
public:
  int getA() { return a; }
};

//[[deprecated]] void func(A &a) {
//[[friend(int)]] void func(A &a) {
//__attribute__((vec_type_hint(A))) void func(A &a) {
template <int I>
void func(A &a);
template <int I>
__attribute__((friend(A))) void func(A &a) {
  a.a = 1;
}

int main() {
  A a;
  assert(a.getA() == 0);
  func<42>(a);
  assert(a.getA() == 1);
}

// TODO Why [[friend(A)]] is not working?
// TODO Why the lookup is failing when no foward decl before?
// void func(A &a);
//
