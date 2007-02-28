#include "menu_config.h"
#include "mrt/serializator.h"
#include "mrt/chunk.h"
#include "mrt/logger.h"
#include "mrt/random.h"

void SlotConfig::serialize(mrt::Serializator &s) const {
	s.add(type);
	s.add(vehicle);
}

void SlotConfig::deserialize(const mrt::Serializator &s) {
	s.get(type);
	s.get(vehicle);
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
		slots.resize(2);
		slots[0].type = "player";
		slots[0].vehicle = vehicles[mrt::random(3)];
		slots[1].type = "ai";
		slots[1].vehicle = vehicles[mrt::random(3)];
	}
	config = slots;
}

void IMenuConfig::update(const std::string &map, const std::string &variant, const int idx, const SlotConfig &slot) {
	_config[map][variant][idx] = slot;
}

void IMenuConfig::save() {
	mrt::Chunk data;
	serialize2(data);
	std::string hex;
	size_t n = data.getSize();
	unsigned char *ptr = (unsigned char *)data.getPtr();
	for(size_t i = 0; i < n; ++i) {
		hex += mrt::formatString("%02x", ptr[i]);
	}
	LOG_DEBUG(("hex view: %s", hex.c_str()));
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
		
	}
}
