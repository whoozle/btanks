#include "sl08.h"


sl08::signal1<void, int> x;

void test() {
	x.emit(1);
}
