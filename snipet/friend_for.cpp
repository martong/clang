class A {
	struct X{};
	int x = 0;
	int y = 0;
	//__attribute__((friend_for(&A::x,X{}))) friend void func(A& a);
	__attribute__((friend_for(&A::x))) friend void func(A& a);
};

void func(A& a) {
	a.x = 1;
	a.y = 1; // This should not compile
}
