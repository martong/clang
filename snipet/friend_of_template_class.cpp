namespace test2 {

template <class T>
class A {
  int a = 0;
public:
  int getA() { return a; }
};

template class A<int>;

void func(A<int> &a);
__attribute__((friend(A<int>))) void func(A<int> &a) {
  a.a = 1;
}

} // namespace test2
