#include "context.h"
#include "mrt/exception.h"

int main() {
	TRY {
		clunk::Context context;
		context.init(22050, 2, 1024);
		context.deinit();
	} CATCH("main", return 1); 

	return 0;
}
