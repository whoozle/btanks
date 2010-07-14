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

#include "config.h"
#include "mrt/exception.h"
#include "mrt/file.h"
#include "utils.h"
#include <assert.h>
#include <stdlib.h>
#include "mrt/serializable.h"
#include "mrt/serializator.h"

#include "var.h"
#include "console.h"

IMPLEMENT_SINGLETON(Config, IConfig);

IConfig::IConfig() {}

void IConfig::load(const std::string &file) {
	LOG_DEBUG(("loading config from %s", file.c_str()));
	_file = file;
	TRY {
		parse_file(file);
	} CATCH("load", {}); 
	on_console_slot.assign(this, &IConfig::onConsole, Console->on_command);
}

void IConfig::save() const {
	if (_file.empty())
		return;
	LOG_DEBUG(("saving config to %s...", _file.c_str()));	
	std::string data = "<config>\n";
	for(VarMap::const_iterator i = _map.begin(); i != _map.end(); ++i) {
		data += mrt::format_string("\t<value name=\"%s\" type=\"%s\">%s</value>\n", 
			XMLParser::escape(i->first).c_str(), 
			i->second->type.c_str(), 
			XMLParser::escape(i->second->toString()).c_str());
	}
	data += "</config>\n";
	mrt::File f;
	f.open(_file, "wt");
	f.write_all(data);
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

void IConfig::cdata(const std::string &data) {
	if (_name.empty())
		return;
	
	_data += data;
}

const bool IConfig::has(const std::string &name) const {
	if (_temp_map.find(name) != _temp_map.end())
		return true;

	return (_map.find(name) != _map.end());
}

void IConfig::get(const std::string &name, float &value, const float default_value) {
	VarMap::iterator t_i = _temp_map.find(name);
	if (t_i != _temp_map.end()) { //override found
		t_i->second->check("float");
		value = t_i->second->f;
		return;
	}

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
	VarMap::iterator t_i = _temp_map.find(name);
	if (t_i != _temp_map.end()) { //override found
		t_i->second->check("int");
		value = t_i->second->i;
		return;
	}

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
	VarMap::iterator t_i = _temp_map.find(name);
	if (t_i != _temp_map.end()) { //override found
		t_i->second->check("bool");
		value = t_i->second->b;
		return;
	}

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
	VarMap::iterator t_i = _temp_map.find(name);
	if (t_i != _temp_map.end()) { //override found
		t_i->second->check("string");
		value = t_i->second->s;
		return;
	}
	
	VarMap::iterator i = _map.find(name); 
	if (i == _map.end()) {
		_map[name] = new Var("string");
		value = _map[name]->s = default_value;
	} else {
		i->second->check("string");
		value = i->second->s;
	}
}

void IConfig::set(const std::string &name, const float value) {
	Var *v = _map[name];
	if (v == NULL) {
		v = _map[name] = new Var("float");
	} else v->type = "float";
	v->f = value;
}

void IConfig::set(const std::string &name, const std::string &value) {
	Var *v = _map[name];
	if (v == NULL) {
		v = _map[name] = new Var("string");
	} else v->type = "string";
	v->s = value;
}

void IConfig::set(const std::string &name, const int value) {
	Var *v = _map[name];
	if (v == NULL) {
		v = _map[name] = new Var("int");
	}
	v->i = value;
}

void IConfig::set(const std::string &name, const bool value) {
	Var *v = _map[name];
	if (v == NULL) {
		v = _map[name] = new Var("bool");
	}
	v->b = value;
}

void IConfig::remove(const std::string &name) {
	_map.erase(name);
}

void IConfig::rename(const std::string &old_name, const std::string &new_name) {
	if (old_name == new_name)
		return;
	
	VarMap::iterator i = _map.find(old_name);
	if (i != _map.end()) {
		_map[new_name] = i->second;
		_map.erase(i);
	}
}

void IConfig::registerInvalidator(bool *ptr) {
	_invalidators.insert(ptr);
}

void IConfig::setOverride(const std::string &name, const Var &var) {
	LOG_DEBUG(("adding override for '%s'", name.c_str()));
	Var * v = _temp_map[name];
	if (v == NULL) 
		_temp_map[name] = new Var(var);
	else 
		*v = var;	
}

void IConfig::deserializeOverrides(const mrt::Serializator &s) {
	throw_ex(("implement me"));
	invalidateCachedValues();
}

void IConfig::clearOverrides() {
	LOG_DEBUG(("clearing %u overrides...", (unsigned)_temp_map.size()));
	std::for_each(_temp_map.begin(), _temp_map.end(), delete_ptr2<VarMap::value_type>());
	_temp_map.clear();
}

void IConfig::invalidateCachedValues() {
	LOG_DEBUG(("invalidating %u cached values (%u overrides)...", (unsigned)_invalidators.size(), (unsigned)_temp_map.size()));
	for(std::set<bool *>::iterator i = _invalidators.begin(); i != _invalidators.end(); ++i) {
		*(*i) = false;
	}
}

const std::string IConfig::onConsole(const std::string &cmd, const std::string &param) {
	if (cmd != "set") 
		return std::string();
		
	try {
		std::vector<std::string> par;
		mrt::split(par, param, " ", 3);
		if (par.size() < 3 || par[0].empty() || par[1].empty() || par[2].empty())
			return "usage: set [int|string|bool] name value";
		
		Var v(par[0]);
		v.fromString(par[2]);
		const std::string &name = par[1];
		Var * var = _map[name];
		if  (var == NULL)
			var = _map[name] = new Var(v);
		else 
			*var = v;
		invalidateCachedValues();
	} catch(std::exception &e) {
		return std::string("error") + e.what();
	}
	return "ok";
}


IConfig::~IConfig() {
	LOG_DEBUG(("cleaning up config..."));
	std::for_each(_temp_map.begin(), _temp_map.end(), delete_ptr2<VarMap::value_type>());
	std::for_each(_map.begin(), _map.end(), delete_ptr2<VarMap::value_type>());
}

void IConfig::enumerateKeys(std::set<std::string> &keys, const std::string &pattern) const {
	keys.clear();
	for(VarMap::const_iterator i = _temp_map.begin(); i != _temp_map.end(); ++i) {
		if (i->first.compare(0, pattern.size(), pattern) == 0)
			keys.insert(i->first);
	}
	//copy-paste ninja was here.... again...
	for(VarMap::const_iterator i = _map.begin(); i != _map.end(); ++i) {
		if (i->first.compare(0, pattern.size(), pattern) == 0)
			keys.insert(i->first);
	}
}
