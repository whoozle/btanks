#include "mrt/logger.h"
#include "mrt/exception.h"
#include "game.h"

int main(int argc, const char **argv) {
	TRY {
		LOG_DEBUG(("starting up..."));
		Game->init(argc, argv);
		TRY {
			Game->run();
		} CATCH("run", {});
		Game->deinit();
		
	} CATCH("main", {return 1;})
	return 0;
}
