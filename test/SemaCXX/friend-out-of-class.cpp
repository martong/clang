// RUN: %clang_cc1 -std=c++1y -fsyntax-only -verify %s
// expected-no-diagnostics

// out-of-class friend function
namespace test0 {

class A {
  int a = 0;
public:
  int getA() { return a; }
};

void func(A &a);
__attribute__((friend(A))) void func(A &a) {
  a.a = 1;
}

}


namespace test1 {

class A {
  int a = 0;
public:
  int getA() { return a; }
};

template <int I>
void func(A &a);

template <int I>
__attribute__((friend(A))) void func(A &a) {
  a.a = 1;
}

template void func<0>(A&);

}
