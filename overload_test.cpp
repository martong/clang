void f();
void f(int);

namespace N {
    struct S {
    };
    void f(S*);
    void f();
    template <typename T>
    void f(T);
}

void usage() {
    N::S* s = nullptr;
    f(s);
}
