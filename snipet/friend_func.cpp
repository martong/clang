class A {
  int a;
  friend void func(A &);
};
void func(A &a) {
  a.a = 1;
};
