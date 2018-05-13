#include "foo.h"

int main() {
    // Bar::f() from test double,
    // Bar::g() from production code
    return Foo().ff() == 15 ? 0 : 1;
}
