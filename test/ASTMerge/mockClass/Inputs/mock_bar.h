#include "mock_bar_modifiers_fwd.h"

struct [[test_double]] Bar {
    int f_return_value = 0;
    int f() {
        return f_return_value;
    }
};

void set_f_return_value(Bar* bar, int value) {
    bar->f_return_value = value;
}
