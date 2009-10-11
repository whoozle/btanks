#ifndef __BTANKS_CONFIG_H__
#define __BTANKS_CONFIG_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
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


#include "mrt/singleton.h"
#include "mrt/xml.h"
#include <string>
#include <map>
#include <set>
#include "sl08/sl08.h"
#include "export_btanks.h"

class Var;
namespace mrt {
	class Serializator;
}

class BTANKSAPI IConfig : public mrt::XMLParser {
public:
	DECLARE_SINGLETON(IConfig);
	IConfig();
	void load(const std::string &file);
	void save() const;

	const bool has(const std::string &name) const;

	void get(const std::string &name, float &value, const float default_value);
	void get(const std::string &name, int &value, const int default_value);
	void get(const std::string &name, bool &value, const bool default_value);
	void get(const std::string &name, std::string &value, const std::string& default_value);
	
	//dont use set(...) in conjunction with GET_CONFIG_VALUE macro
	void set(const std::string &name, const bool value);
	void set(const std::string &name, const int value);
	void set(const std::string &name, const float value);
	void set(const std::string &name, const std::string &value);
	
	void remove(const std::string &name);
	void rename(const std::string &old_name, const std::string &new_name);
	
	~IConfig();
	
	void setOverride(const std::string &name, const Var &var);
	void deserializeOverrides(const mrt::Serializator &s);
	void clearOverrides();
	void invalidateCachedValues();
	
	void registerInvalidator(bool *ptr);
	
	void enumerateKeys(std::set<std::string> &keys, const std::string &pattern) const;
	
private: 
	
	virtual void start(const std::string &name, Attrs &attr);
	virtual void end(const std::string &name);
	virtual void cdata(const std::string &data);
	
	sl08::slot2<const std::string, const std::string &, const std::string &, IConfig> on_console_slot;
	const std::string onConsole(const std::string &cmd, const std::string &param);

	typedef std::map<const std::string, Var*> VarMap;

	std::string _file;

	VarMap _map, _temp_map;
	
	std::string _name, _type, _data;
	
	std::set<bool *> _invalidators;
};

PUBLIC_SINGLETON(BTANKSAPI, Config, IConfig);

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

