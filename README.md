The goal of this branch is to add a new attribute which implements the "out-of-class friend" language extension.
Example, the following would compile, because `func` is marked as friend function of `class A`:
```
class A {
  int a = 0;
public:
  int getA() { return a; }
};

void func(A &a);
__attribute__((friend(A))) void func(A &a) {
  a.a = 1;
}
```
Currently only friend functions and function templates can be added this way.
Later it might be extended to be able to add out-of-class friend classes and friend class templates.

#### diff
https://github.com/martong/clang/compare/friendfor_0...martong:out-of-class_friend_attr
#### llvm
Use friendfor_0 tag from https://github.com/martong/llvm/releases
