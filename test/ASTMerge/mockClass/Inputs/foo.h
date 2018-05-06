class Bar {
    int a;
public:
    int f() {
        return 1;
    }
};

class Foo {
public:
    Bar bar;
    int ff() {
        return bar.f();
    }
};
