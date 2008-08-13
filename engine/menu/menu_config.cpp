
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
#include "menu_config.h"
#include "mrt/serializator.h"
#include "mrt/chunk.h"
#include "mrt/logger.h"
#include "mrt/random.h"
#include "mrt/b64.h"
#include "config.h"
#include <string.h>

IMPLEMENT_SINGLETON(MenuConfig, IMenuConfig);

void SlotConfig::serialize(mrt::Serializator &s) const {
	s.add(type);
	s.add(vehicle);
}

void SlotConfig::deserialize(const mrt::Serializator &s) {
	s.get(type);
	s.get(vehicle);
}

#ifdef _WINDOWS
#	define strcasecmp _stricmp
#endif

const bool SlotConfig::hasType(const std::string &t) const {
	return strcasecmp(type.c_str(), t.c_str()) == 0;
}


const bool IMenuConfig::empty(const std::string &map, const std::string &variant) const {
	ConfigMap::const_iterator i = _config.find(map);
	if (i == _config.end())
		return true;
	VariantMap::const_iterator j = i->second.find(variant);
	if (j == i->second.end())
		return true;
	return j->second.empty();
}

void IMenuConfig::fill(const std::string &map, const std::string &variant, std::vector<SlotConfig> &config) {
	if (empty(map, variant)) {
		fillDefaults(map, variant, config);
	} else {
		config = _config[map][variant];
	}
}


void IMenuConfig::fillDefaults(const std::string &map, const std::string &variant, std::vector<SlotConfig> &config) {
	config.clear();
	std::vector<SlotConfig> &slots = _config[map][variant];
	slots.clear();
	static const char *vehicles[] = {"tank", "shilka", "launcher"};
	
	if (variant == "split") {
		slots.resize(2);
		slots[0].type = "player-1";
		slots[0].vehicle = vehicles[mrt::random(3)];
		slots[1].type = "player-2";
		slots[1].vehicle = vehicles[mrt::random(3)];
	} else {
		slots.resize(1);
		slots[0].type = "player";
		slots[0].vehicle = vehicles[mrt::random(3)];
		//slots[1].type = "ai";
		//slots[1].vehicle = vehicles[mrt::random(3)];
	}
	config = slots;
}

void IMenuConfig::update(const std::string &map, const std::string &variant, const int idx, const SlotConfig &slot) {
	std::vector<SlotConfig> &slots = _config[map][variant];
	if (idx >= (int)slots.size())
		slots.resize(idx + 1);

	slots[idx] = slot;
}

void IMenuConfig::load(const int mode) {
	save();
	this->mode = mode;
TRY {
	mrt::Chunk data;
	std::string src;
	Config->get(mrt::format_string("menu.mode-%d.state", mode), src, std::string());
	if (src.empty())
		return;
	mrt::Base64::decode(data, src);
	//data.reserve(3); //bug in base64 ?
	deserialize2(data);
} CATCH("load", {});
}


void IMenuConfig::save() {
	if (mode < 0)
		return;
	
	mrt::Chunk data;
	serialize2(data);
	std::string dump;
	mrt::Base64::encode(dump, data);
	//LOG_DEBUG(("dump: %s", dump.c_str()));
	Config->set(mrt::format_string("menu.mode-%d.state", mode), dump);
}

void IMenuConfig::serialize(mrt::Serializator &s) const {
	s.add((int)_config.size());
	for(ConfigMap::const_iterator i = _config.begin(); i != _config.end(); ++i) {
		s.add(i->first);
		const VariantMap &vmap = i->second;
		s.add((int)vmap.size());
		for(VariantMap::const_iterator j = vmap.begin(); j != vmap.end(); ++j) {
			s.add(j->first);
			const std::vector<SlotConfig> &slots = j->second;
			s.add((int)slots.size());
			for(size_t k = 0; k < slots.size(); ++k) {
				slots[k].serialize(s);
			}
		}
	}
}

void IMenuConfig::deserialize(const mrt::Serializator &s) {
	_config.clear();
	int n;
	s.get(n);
	while(n--) {
		std::string map;
		s.get(map);
		VariantMap &vmap = _config[map];

		int vn;
		s.get(vn);
		while(vn--) {
			std::string variant;
			s.get(variant);

			std::vector<SlotConfig> &slots = vmap[variant];
			int sn;
			s.get(sn);
			
			slots.resize(sn);
			for(int i = 0; i < sn; ++i) {
				slots[i].deserialize(s);
			}
		}
	}
}
