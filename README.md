The goal of this branch is to add a new attribute which implements the "selective friend" language extension.
Example, the following would not compile, because `func` is restriced to have acces to private member `x` only:
```
class A {
  int x = 0;
  int y = 0; // expected-note {{implicitly declared private here}}
  __attribute__((friend_for(&A::x))) friend void func(A &a);
};

void func(A &a) {
  a.x = 1;
  a.y = 1; // expected-error {{'y' is a private member of 'A'}}
}
```
Currently only one instance member can be added as an argument to the attribute.
Later it might be extended have more arguments, and to handle member classes and static variables as well.
