#include "foo.h"
#include "mock_bar_modifiers_fwd.h"

int main() {
    Foo foo;
    set_f_return_value(&foo.bar, 13);
    return foo.ff() == 13 ? 0 : 1;
}
