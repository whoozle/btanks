#ifndef BTANKS_MENU_CONFIG_H__
#define BTANKS_MENU_CONFIG_H__

#include <string>
#include <vector>
#include <map>
#include "mrt/serializable.h"
#include "mrt/singleton.h"

struct SlotConfig : public mrt::Serializable {
	std::string type, vehicle;

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);
	
};

class IMenuConfig : public mrt::Serializable {
public: 
	DECLARE_SINGLETON(IMenuConfig);

	const bool empty(const std::string &map, const std::string &variant) const;
	void fill(const std::string &map, const std::string &variant, std::vector<SlotConfig> &config);
	void fillDefaults(const std::string &map, const std::string &variant, std::vector<SlotConfig> &config);
	void update(const std::string &map, const std::string &variant, const int idx, const SlotConfig &slot);

	void save();
	void load();

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);

private:

	typedef std::map<const std::string, std::vector<SlotConfig> > VariantMap;
	typedef std::map<const std::string, VariantMap> ConfigMap;
	ConfigMap _config;

};

SINGLETON(MenuConfig, IMenuConfig);

#endif

