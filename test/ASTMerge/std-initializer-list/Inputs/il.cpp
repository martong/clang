namespace std {
  template<typename T> struct initializer_list {
    const T *begin, *end;
    initializer_list();
  };
}

std::initializer_list<int> IL = {1, 2, 3, 4};
