class Bar {
    int a;
public:
    int f() {
        return 1;
    }
};

class Foo {
    Bar bar;
public:
    int ff() {
        return bar.f();
    }
};
