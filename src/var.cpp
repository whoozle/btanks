
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
#include "var.h"
#include "mrt/exception.h"
#include "mrt/serializator.h"
#include <assert.h>

void Var::serialize(mrt::Serializator &s) const {
		if (type.empty()) 
			throw_ex(("cannot serialize empty variable"));
		int t = type[0];
		s.add(t);
		if (t == 'i') 
			s.add(i);
		else if (t == 'b') 
			s.add(b);
		else if (t == 's') 
			s.add(this->s);
		else if (t == 'f') 
			s.add(f);
	}

void Var::deserialize(const mrt::Serializator &s) {
		int t;
		s.get(t);
		switch(t) {
			case 'i': 
				type = "int";
				s.get(i);
			break;
			case 'b': 
				type = "bool";
				s.get(b);
			break;
			case 's': 
				type = "string";
				s.get(this->s);
			break;
			case 'f': 
				type = "float";
				s.get(f);
			break;
			default:
				throw_ex(("unknown type %02x recv'ed", t));
		}
	}
	
	

void Var::check(const std::string &t) const {
		if (type != t)
			throw_ex(("invalid type requested(%s), real type: %s", t.c_str(), type.c_str()));
	}
	
const std::string Var::toString() const {
		assert(!type.empty());
		if (type == "int")
			return mrt::formatString("%d", i);
		else if (type == "bool") 
			return b?"true":"false";
		else if (type == "float") 
			return mrt::formatString("%g", f);
		else if (type == "string") 
			return mrt::formatString("%s", s.c_str());
		throw_ex(("cannot convert %s to string", type.c_str()));
		return "";//stub
	}

void Var::fromString(const std::string &str) {
		assert(!type.empty());
		
		if (type == "int")
			i = atoi(str.c_str());
		else if (type == "bool") {
			if (str == "true") {
				b = true;
			} else if (str == "false") {
				b = false;
			} else throw_ex(("'%s' used as boolean value.", str.c_str()));
		} else if (type == "float") 
			f = atof(str.c_str());
		else if (type == "string") 
			s = str;
		else throw_ex(("cannot construct %s from string", type.c_str()));
	}
