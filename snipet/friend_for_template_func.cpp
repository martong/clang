class A;
template <int I>
void funcT(A& a);

class A {
	struct X{};
	int x = 0;
	int y = 0;

	template <int I>
	__attribute__((friend_for(&A::x))) friend void funcT(A& a) {
		a.y = 1; // This should not compile
	}
};

void foo() {
	A a;
	funcT<0>(a);
}
