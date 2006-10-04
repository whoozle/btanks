#include "config.h"
#include "mrt/exception.h"
#include "mrt/file.h"
#include "utils.h"
#include <assert.h>
#include <stdlib.h>

IMPLEMENT_SINGLETON(Config, IConfig)


class IConfig::Var {
public: 
	std::string type;
	Var() {}
	Var(const std::string & type): type(type) {}
	
	void check(const std::string &t) const {
		if (type != t)
			throw_ex(("invalid type requested(%s), real type: %s", t.c_str(), type.c_str()));
	}
	
	const std::string toString() const {
		assert(!type.empty());
		if (type == "int")
			return mrt::formatString("%d", i);
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
		else if (type == "float") 
			f = atof(str.c_str());
		else if (type == "string") 
			s = str;
		else throw_ex(("cannot construct %s from string", type.c_str()));
	}
	
	int i;
	float f;
	std::string s;
};

void IConfig::load(const std::string &file) {
	_file = file;
	TRY {
		parseFile(file);
	} CATCH("load", {}); 
}

void IConfig::save() const {
	if (_file.empty())
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
		v.fromString(_data);
	} CATCH("fromString", return;);

	LOG_DEBUG(("read config value %s of type %s (%s)", _name.c_str(), _type.c_str(), _data.c_str()));
	VarMap::iterator i = _map.find(_name);
	if (i == _map.end()) {
		_map[_name] = new Var(v);
	} else {
		delete i->second;
		i->second = new Var(v);
	}
	_name.clear();
}

void IConfig::charData(const std::string &data) {
	if (_name.empty())
		return;
	
	_data = data;
	mrt::trim(_data);
}

void IConfig::get(const std::string &name, float &value, const float& default_value) {
	VarMap::iterator i = _map.find(name); 
	if (i == _map.end()) {
		_map[name] = new Var("float");
		_map[name]->f = default_value;
	} else {
		i->second->check("float");
	}
	value = _map[name]->f;
}
void IConfig::get(const std::string &name, int &value, const int& default_value) {
	VarMap::iterator i = _map.find(name); 
	if (i == _map.end()) {
		_map[name] = new Var("int");
		_map[name]->i = default_value;
	} else {
		i->second->check("int");
	}
	value = _map[name]->i;
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

IConfig::~IConfig() {
	std::for_each(_map.begin(), _map.end(), delete_ptr2<VarMap::value_type>());
}
