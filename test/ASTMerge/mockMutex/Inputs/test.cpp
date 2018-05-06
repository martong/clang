#include "Entity.hpp"

int main() {
    Entity e;
    return e.process(1) == -1 ? 0 : 1;
}
