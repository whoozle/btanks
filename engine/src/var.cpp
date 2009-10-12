
/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
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

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/
#include "var.h"
#include "mrt/exception.h"
#include "mrt/serializator.h"
#include <assert.h>
#include <stdlib.h>

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
			return mrt::format_string("%d", i);
		else if (type == "bool") 
			return b?"true":"false";
		else if (type == "float") 
			return mrt::format_string("%g", f);
		else if (type == "string") 
			return mrt::format_string("%s", s.c_str());
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
