#include "sl08.h"
#include <stdio.h>

sl08::signal1<void, int> signal;

class Foo {
	sl08::slot1<void, int, Foo> test_slot;
public: 

	Foo() : test_slot(this, &Foo::test) {
		test_slot.connect(signal);
	}
	void test(int x) {
		printf("test(%d)\n", x);
	}
};


int main(const int argc, const char *argv[]) {
	Foo foo;
	signal.emit(1);
}
