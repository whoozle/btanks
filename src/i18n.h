#ifndef BTANKS_I18N_H__
#define BTANKS_I18N_H__

#include "mrt/xml.h"
#include <map>
#include <set>
#include <deque>
#include <string>


struct lessnocase
{
	bool operator()(const std::string& s1, const std::string& s2) const {
#ifdef WIN32
		return _stricmp(s1.c_str(), s2.c_str()) < 0;
#endif
		return strcasecmp(s1.c_str(), s2.c_str()) < 0;
	}
};


class II18n : public mrt::XMLParser {
public:
	DECLARE_SINGLETON(II18n);
	II18n();

	void load(const std::string &file, const std::string &language);
	
	const std::string& get(const std::string &area, const std::string &message) const;
	const bool has(const std::string &area, const std::string &message) const;
	
private: 

	virtual void start(const std::string &name, Attrs &attr);
	virtual void end(const std::string &name);
	virtual void charData(const std::string &data);
	
	std::deque<std::string> _path;

	typedef std::map<const std::string, std::string, lessnocase> Strings;
	std::string _lang, _string_id, _string_lang, _cdata;
	Strings _strings;
	
	std::set<std::string> _unlocalized;
};

SINGLETON(I18n, II18n);

#endif
