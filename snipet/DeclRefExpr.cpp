struct A {
	int x;
};

void f(A& a) {
	(&A::x);
}
