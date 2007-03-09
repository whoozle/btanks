#ifndef __BTANKS_CONFIG_H__
#define __BTANKS_CONFIG_H__

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


#include "mrt/singleton.h"
#include "mrt/serializable.h"
#include "mrt/xml.h"
#include <string>
#include <map>
#include <set>

class Var;

class IConfig : public mrt::XMLParser, mrt::Serializable {
public:
	DECLARE_SINGLETON(IConfig);
	IConfig();
	void load(const std::string &file);
	void save() const;

	void get(const std::string &name, float &value, const float default_value);
	void get(const std::string &name, int &value, const int default_value);
	void get(const std::string &name, bool &value, const bool default_value);
	void get(const std::string &name, std::string &value, const std::string& default_value);
	
	//dont use set(...) in conjunction with GET_CONFIG_VALUE macro
	void set(const std::string &name, const bool value);
	void set(const std::string &name, const int value);
	void set(const std::string &name, const float value);
	void set(const std::string &name, const std::string &value);
	~IConfig();
	
	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);

	void setOverride(const std::string &name, const Var &var);
	void deserializeOverrides(const mrt::Serializator &s);
	void clearOverrides();
	void invalidateCachedValues();
	
	void registerInvalidator(bool *ptr);
private: 
	
	virtual void start(const std::string &name, Attrs &attr);
	virtual void end(const std::string &name);
	virtual void charData(const std::string &data);
	const std::string onConsole(const std::string &cmd, const std::string &param);

	typedef std::map<const std::string, Var*> VarMap;

	std::string _file;

	VarMap _map, _temp_map;
	
	std::string _name, _type, _data;
	
	std::set<bool *> _invalidators;
};

SINGLETON(Config, IConfig);


#define GET_CONFIG_VALUE(name, type, value, default_value) \
	type value; \
	{ \
		static bool i; \
		static type v; \
		if (!i) { \
			Config->registerInvalidator(&i); \
			Config->get(name, v, default_value); \
			i = true; \
		} \
		value = v; \
	}

#endif

