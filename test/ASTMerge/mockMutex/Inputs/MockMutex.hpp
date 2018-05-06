namespace std {

struct [[test_double]] mutex {
    bool try_lock() { return false; }
};

}
