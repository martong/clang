template <typename T>
struct Y {};

void f1(int, Y<int> &y) {}

template <>
struct Y<int> {
  void f2() {
    Y<int> a;
    f1(0, a);
  }
};
