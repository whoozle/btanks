#ifndef __BTANKS_CONFIG_H__
#define __BTANKS_CONFIG_H__

#include "mrt/singleton.h"
#include "mrt/xml.h"
#include <string>
#include <map>

class IConfig : public mrt::XMLParser {
public:
	DECLARE_SINGLETON(IConfig);
	void load(const std::string &file);
	void save() const;

	void get(const std::string &name, float &value, const float& default_value);
	void get(const std::string &name, int &value, const int& default_value);
	void get(const std::string &name, bool &value, const bool& default_value);
	void get(const std::string &name, std::string &value, const std::string& default_value);
	~IConfig();
private: 
	class Var;
	
	virtual void start(const std::string &name, Attrs &attr);
	virtual void end(const std::string &name);
	virtual void charData(const std::string &data);

	typedef std::map<const std::string, Var*> VarMap;

	std::string _file;

	VarMap _map;
	
	std::string _name, _type, _data;
};

SINGLETON(Config, IConfig);


#define GET_CONFIG_VALUE(name, type, value, default_value) \
	type value; \
	{ \
		static bool i; \
		static type v; \
		if (!i) { \
			i = true; \
			Config->get(name, v, default_value);	\
		} \
		value = v; \
	}

#endif

