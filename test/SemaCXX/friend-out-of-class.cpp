// RUN: %clang_cc1 -std=c++1y -fsyntax-only -verify %s
// expected-no-diagnostics

// out-of-class friend function
namespace test0 {

class A {
  int a = 0;
public:
  int getA() { return a; }
};

__attribute__((friend(A))) void func(A &a) {
  a.a = 1;
}

// The above friend declaration is equivalent if it would be declared in-class.
// Therefore is not found by normal lookup.
// So, we need to make it available by declaring it as a free function,
// just as we'd do it with in-class declarations.
void func(A &a);
void user() {
  A a;
  func(a);
}

} // namespace test0


namespace test1 {

class A {
  int a = 0;
public:
  int getA() { return a; }
};

template <int I>
__attribute__((friend(A))) void func(A &a) {
  a.a = 1;
}

template <int I>
void func(A &a);
void user() {
  A a;
  func<0>(a);
}

} // namespace test1


// Befriending class template
namespace test2 {

template <class T>
class A {
  int a = 0;
public:
  int getA() { return a; }
};

// Explicit instantiation is needed,
// since the attribute accesses the instantiation.
// (It accesses the 'DataDefinition' of the 'CXXRecordDecl' of the instantiation.)
// TODO Trigger the explicit instantiation from the attribute semantic action!
template class A<int>;

void func(A<int> &a);

__attribute__((friend(A<int>))) void func(A<int> &a) {
  a.a = 1;
}

void func(A<int> &a);
void user() {
  A<int> a;
  func(a);
}

} // namespace test2
