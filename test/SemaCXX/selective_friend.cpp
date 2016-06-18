// RUN: %clang_cc1 -std=c++1y -fsyntax-only -verify %s

// test function with attr
namespace test0 {
class A {
  struct X {};
  int x = 0;
  int y = 0; // expected-note {{implicitly declared private here}}
  __attribute__((friend_for(&A::x))) friend void func(A &a);
};

void func(A &a) {
  a.x = 1;
  a.y = 1; // expected-error {{'y' is a private member of 'test0::A'}}
}
}

// regression test of regular friend function
namespace test1 {
class A {
  struct X {};
  int x = 0;
  int y = 0;
  friend void func(A &a);
};

void func(A &a) {
  a.x = 1;
  a.y = 1;
}
}

// function template with attr
namespace test2 {
class A;
template <int I>
void funcT(A &a);

class A {
  struct X {};
  int x = 0;
  int y = 0; // expected-note {{implicitly declared private here}}

  template <int I>
  __attribute__((friend_for(&A::x))) friend void funcT(A &a);
};

template <int I>
void funcT(A &a) {
  a.y = 1; // expected-error {{'y' is a private member of 'test2::A'}}
}

template void funcT<0>(A &);
}

// regression test of regular friend function template
namespace test3 {
class A;
template <int I>
void funcT(A &a);

class A {
  struct X {};
  int x = 0;
  int y = 0;

  template <int I>
  friend void funcT(A &a);
};

template <int I>
void funcT(A &a) {
  a.y = 1;
}

template void funcT<0>(A &);
}

// select member function with attr
namespace test4 {
class A {
  void x() {};
  int y = 0; // expected-note {{implicitly declared private here}}
  __attribute__((friend_for(&A::x))) friend void func(A &a);
};

void func(A &a) {
  a.x();
  a.y = 1; // expected-error {{'y' is a private member of 'test4::A'}}
}
}
