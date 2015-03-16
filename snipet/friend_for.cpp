class A {
	int x = 0;
	__attribute__((friend_for(x))) friend void func(A& a);
};

void func(A& a) {
	a.x = 1;
}
