#ifndef __BTASKS_SOUND_AL_EX_H__
#define __BTASKS_SOUND_AL_EX_H__

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

#include "mrt/exception.h"
#include <AL/al.h>
#include <AL/alut.h>

class ALException : public mrt::Exception { 
public:
	ALException(const ALenum code);
	virtual ~ALException() throw() {};
	const std::string getCustomMessage();
private: 
	ALenum _code;
}; 

class ALUTException : public mrt::Exception { 
public:
	ALUTException(const ALenum code);
	virtual ~ALUTException() throw() {};
	const std::string getCustomMessage();
private: 
	ALenum _code;
}; 

#define throw_al(r, str) throw_generic_no_default(ALException, str, (r));
#define throw_alut(r, str) throw_generic_no_default(ALUTException, str, (r));

#define AL_CHECK(fmt) { ALenum r; \
	if ((r = alGetError()) != AL_NO_ERROR) \
	throw_al(r, fmt); \
}

#define AL_CHECK_NON_FATAL(fmt) { ALenum r; \
	if ((r = alGetError()) != AL_NO_ERROR) \
		LOG_ERROR(("%s: error %08x", (mrt::formatString fmt ).c_str(), (unsigned)r)); \
}

#define ALUT_CHECK(fmt) { ALenum r; \
	if ((r = alutGetError()) != AL_NO_ERROR) \
	throw_alut(r, fmt); \
}

#endif

