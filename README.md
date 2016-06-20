The goal of this branch is to add a new attribute which implements the "selective friend" language extension.
Example, the following would not compile, because `func` is restriced to have access to private member `x` only:
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
Later it might be extended to have more arguments, and to handle member classes and static variables as well.

#### diff
https://github.com/martong/clang/compare/friendfor_0...martong:selective_friend
#### llvm
Use friendfor_0 tag from https://github.com/martong/llvm/releases
### tests
Run the selective friend tests like this:
```
python PATH_TO_LLVM/llvm/utils/lit/lit.py -sv --param clang_site_config=PATH_TO_CLANG/clang/test/lit.site.cfg PATH_TO_CLANG/clang/test/SemaCXX/selective_friend.cpp
```
Run the whole regression test suite for C++ sema:
```
python path_to_llvm_SRC/llvm/utils/lit/lit.py -sv --param clang_site_config=path_to_clang_BUILD/clang/test/lit.site.cfg path_to_clang_SRC/clang/test/SemaCXX/
```
### measurements
This is how I created the charts:
```
mkdir M1000.SF1000
./runTest.py -pc ~w/friendfor/build.selective_friend.release/bin/clang++ -M 1000 -SF 1000 -s 1 > M1000.SF1000/selective
./runTest.py -pc ~w/friendfor/build.selective_friend.release/bin/clang++ -M 1000 -SF 1000 > M1000.SF1000/new
./runTest.py -pc ~w/friendfor/build.friendfor_0.release/bin/clang++ -M 1000 -SF 1000 > M1000.SF1000/baseline
cd M1000.SF1000
../plot.py baseline new selective
```
