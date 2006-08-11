/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "mrt/logger.h"
#include "mrt/exception.h"
#include "game.h"
#include "version.h"
#include <stdlib.h>

int main(int argc, const char **argv) {
	TRY {
#ifdef WIN32
		std::string path = getenv("PATH");
		path = "PATH=.\\bin\\;" + path;
		_putenv(path.c_str());
		LOG_DEBUG(("path = %s", path.c_str()));
#endif
		LOG_NOTICE(("starting up... version: %s", getVersion().c_str()));
		Game->init(argc, argv);
		TRY {
			Game->run();
		} CATCH("run", {});
		Game->deinit();
		LOG_DEBUG(("exiting"));
	} CATCH("main", {return 1;})
	return 0;
}
