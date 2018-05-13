class Bar {
    int a;
public:
    int f() {
        return 1;
    }
    int g() {
        return 2;
    }
};

class Foo {
    Bar bar;
public:
    int ff() {
        return bar.f() + bar.g();
    }
};
