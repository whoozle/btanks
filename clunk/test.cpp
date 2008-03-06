#include "context.h"
#include "mrt/exception.h"
#include <stdlib.h>

int main() {
	TRY {
		clunk::Context context;
		context.init(22050, 2, 1024);
		sleep(2);
		context.deinit();
	} CATCH("main", return 1); 

	return 0;
}
