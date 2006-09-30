#include <stdlib.h>
#include "random.h"

const int mrt::random(const int max) {
	return (int) (max * (rand() / (RAND_MAX + 1.0)));
}
