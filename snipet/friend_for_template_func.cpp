class A;
template <int I>
void funcT(A& a);

class A {
	struct X{};
	int x = 0;
	int y = 0;

	//template <int I>
	//__attribute__((friend_for(&A::x))) friend void funcT(A& a) {
		//a.y = 1; // This should not compile
	//}

	template <int I>
	//friend void funcT(A& a);
	__attribute__((friend_for(&A::x))) friend void funcT(A& a);

};

template <int I>
void funcT(A& a) {
	a.y = 1; // This should not compile
}

template void funcT<0>(A&);

