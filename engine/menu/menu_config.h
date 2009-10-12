#ifndef BTANKS_MENU_CONFIG_H__
#define BTANKS_MENU_CONFIG_H__

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

#include <string>
#include <vector>
#include <map>
#include "mrt/serializable.h"
#include "mrt/singleton.h"

struct SlotConfig : public mrt::Serializable {
	std::string type, vehicle;

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);

	const bool hasType(const std::string &t) const;	
};

class IMenuConfig : public mrt::Serializable {
public: 
	DECLARE_SINGLETON(IMenuConfig);
	IMenuConfig() : mode(-1) {}

	const bool empty(const std::string &map, const std::string &variant) const;
	void fill(const std::string &map, const std::string &variant, std::vector<SlotConfig> &config);
	void fillDefaults(const std::string &map, const std::string &variant, std::vector<SlotConfig> &config);
	void update(const std::string &map, const std::string &variant, const int idx, const SlotConfig &slot);

	void save();
	void load(const int mode);

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);

private:
	int mode;

	typedef std::map<const std::string, std::vector<SlotConfig> > VariantMap;
	typedef std::map<const std::string, VariantMap> ConfigMap;
	ConfigMap _config;

};

SINGLETON(MenuConfig, IMenuConfig);

#endif

