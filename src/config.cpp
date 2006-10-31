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

#include "config.h"
#include "mrt/exception.h"
#include "mrt/file.h"
#include "utils.h"
#include <assert.h>
#include <stdlib.h>
#include "mrt/serializable.h"
#include "mrt/serializator.h"

IMPLEMENT_SINGLETON(Config, IConfig)

IConfig::IConfig() : _ro(true) {}


class IConfig::Var : public mrt::Serializable {
public: 
	std::string type;
	Var() {}
	Var(const std::string & type): type(type) {}
	
	virtual void serialize(mrt::Serializator &s) const {
		if (type.empty()) 
			throw_ex(("cannot serialize empty variable"));
		char t = type[0];
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
	virtual void deserialize(const mrt::Serializator &s) {
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
	
	
	void check(const std::string &t) const {
		if (type != t)
			throw_ex(("invalid type requested(%s), real type: %s", t.c_str(), type.c_str()));
	}
	
	const std::string toString() const {
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

	void fromString(const std::string &str) {
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
	
	int i;
	bool b;
	float f;
	std::string s;
};

void IConfig::load(const std::string &file) {
	_file = file;
	TRY {
		parseFile(file);
	} CATCH("load", {}); 
	_ro = false;
}

void IConfig::save() const {
	if (_file.empty() || _ro)
		return;
	LOG_DEBUG(("saving config to %s...", _file.c_str()));	
	std::string data = "<config>\n";
	for(VarMap::const_iterator i = _map.begin(); i != _map.end(); ++i) {
		data += mrt::formatString("\t<value name=\"%s\" type=\"%s\">%s</value>\n", i->first.c_str(), i->second->type.c_str(), i->second->toString().c_str());
	}
	data += "</config>\n";
	mrt::File f;
	f.open(_file, "wt");
	f.writeAll(data);
	f.close();
}


void IConfig::start(const std::string &name, Attrs &attr) {
	if (name != "value") 
		return;
	
	_name = attr["name"];
	_type = attr["type"];
	if (_name.empty() || _type.empty())
		throw_ex(("value tag must contain name and type attrs"));
	
}

void IConfig::end(const std::string &name) {
	if (name != "value") {
		_name.clear();
		return;
	}

	Var v(_type);
	TRY {
		mrt::trim(_data);
		v.fromString(_data);
	} CATCH("fromString", return;);

	//LOG_DEBUG(("read config value %s of type %s (%s)", _name.c_str(), _type.c_str(), _data.c_str()));
	VarMap::iterator i = _map.find(_name);
	if (i == _map.end()) {
		_map[_name] = new Var(v);
	} else {
		delete i->second;
		i->second = new Var(v);
	}
	_name.clear();
	_data.clear();
}

void IConfig::charData(const std::string &data) {
	if (_name.empty())
		return;
	
	_data += data;
}

void IConfig::get(const std::string &name, float &value, const float default_value) {
	VarMap::iterator i = _map.find(name); 
	if (i == _map.end()) {
		_map[name] = new Var("float");
		_map[name]->f = default_value;
	} else {
		i->second->check("float");
	}
	value = _map[name]->f;
}
void IConfig::get(const std::string &name, int &value, const int default_value) {
	VarMap::iterator i = _map.find(name); 
	if (i == _map.end()) {
		_map[name] = new Var("int");
		_map[name]->i = default_value;
	} else {
		i->second->check("int");
	}
	value = _map[name]->i;
}

void IConfig::get(const std::string &name, bool &value, const bool default_value) {
	VarMap::iterator i = _map.find(name); 
	if (i == _map.end()) {
		_map[name] = new Var("bool");
		_map[name]->b = default_value;
	} else {
		i->second->check("bool");
	}
	value = _map[name]->b;
}

void IConfig::get(const std::string &name, std::string &value, const std::string& default_value) {
	VarMap::iterator i = _map.find(name); 
	if (i == _map.end()) {
		_map[name] = new Var("string");
		_map[name]->s = default_value;
	} else {
		i->second->check("string");
	}
	value = _map[name]->s;
}

void IConfig::set(const std::string &name, const std::string &value) {
	Var *v = _map[name];
	if (v == NULL) {
		v = _map[name] = new Var("string");
	}
	v->s = value;
}


void IConfig::setRO(const bool ro) {
	_ro = ro;
}

void IConfig::serialize(mrt::Serializator &s) const {
	int n = _map.size();
	s.add(n);
	for(VarMap::const_iterator i = _map.begin(); i != _map.end(); ++i) {
		s.add(i->first);
		i->second->serialize(s);
	}
}

void IConfig::deserialize(const mrt::Serializator &s) {
	int n;
	s.get(n);
	while(n--) {
		std::string name;
		s.get(name);
		if (_map[name] == NULL)
			_map[name] = new Var;
		_map[name]->deserialize(s);		
	}
}


IConfig::~IConfig() {
	std::for_each(_map.begin(), _map.end(), delete_ptr2<VarMap::value_type>());
}

