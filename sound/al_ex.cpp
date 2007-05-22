
/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
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

#include "al_ex.h"
#include <assert.h>
#include <AL/al.h>

ALException::ALException(const ALenum code): _code(code) {}

const std::string ALException::getCustomMessage() {
	return mrt::formatString("openAL error: %08x", (unsigned)_code);
}

/*
ALUTException::ALUTException(const ALenum code): _code(code) {}

const std::string ALUTException::getCustomMessage() {
	const char * err = alutGetErrorString(_code);
	assert(err != NULL);
	return err;
}
*/
