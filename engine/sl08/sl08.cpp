#include "sl08.h"
#include <string>
#include <stdio.h>

sl08::signal1<const std::string, int> signal;

class Foo {
	sl08::slot1<const std::string, int, Foo> test_slot;
public: 
	Foo() : test_slot(this, &Foo::test) {
		test_slot.connect(signal);
	}
	const std::string test(int x) {
		printf("test(%d)\n", x);
	}
};


int main(const int argc, const char *argv[]) {
	Foo foo;
	signal.emit(1);
}
