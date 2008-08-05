/* sdlx - c++ wrapper for libSDL
 * Copyright (C) 2005-2007 Vladimir Menshakov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "module.h"
#include "sdlx.h"
#include "sdl_ex.h"

using namespace sdlx;

const std::string Module::mangle(const std::string &name) {
#ifdef _WINDOWS
	return name + ".dll";	
#elif defined __APPLE__
	return std::string("lib") + name + ".dylib";
#else 
	return std::string("lib") + name + ".so";
#endif
}

Module::Module() : handle(NULL) {}

void Module::load(const std::string &name) {
	unload();
	handle = SDL_LoadObject(name.c_str());
	if (handle == NULL)
		throw_sdl(("SDL_LoadObject('%s')", name.c_str()));
}

void *Module::sym(const std::string &name) const {
	if (handle == NULL)
		return NULL;
	return SDL_LoadFunction(handle, name.c_str());
}

void Module::unload() {
	if (handle == NULL)
		return;
	SDL_UnloadObject(handle);
	handle = NULL;
}

Module::~Module() {
	unload();
}

void Module::leak() {
	handle = NULL;
}
