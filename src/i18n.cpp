#include "i18n.h"
#include "mrt/exception.h"
#include <assert.h>

IMPLEMENT_SINGLETON(I18n, II18n)

bool lessnocase::operator()(const std::string& s1, const std::string& s2) const {
#ifdef WIN32
		return _stricmp(s1.c_str(), s2.c_str()) < 0;
#else
		return strcasecmp(s1.c_str(), s2.c_str()) < 0;
#endif
}


II18n::II18n() {}

const bool II18n::has(const std::string &_area, const std::string &id) const {
	if (id.empty())
		return false;
		
	Strings::const_iterator i;
	std::string area = _area;
	do {
		i = _strings.find(area + "/" + id);
		if (i != _strings.end())
			return true;
		
		if (area.empty())
			return false;
		
		size_t p = area.rfind('/');
		if (p == area.npos)
			area.clear();
		else 
			area = area.substr(0, p - 1);
	} while (true);
	
	return false;
}


const std::string& II18n::get(const std::string &_area, const std::string &id) const {
	if (id.empty())
		throw_ex(("I18n->get(/empty-id/) is not allowed"));
		
	Strings::const_iterator i;
	std::string area = _area;
	do {
		i = _strings.find(area + "/" + id);
		if (i != _strings.end())
			return i->second;
		
		if (area.empty())
			break;
		
		size_t p = area.rfind('/');
		if (p == area.npos)
			area.clear();
		else 
			area = area.substr(0, p - 1);
	} while (true);
	

	throw_ex(("message with id %s could not be found. (initial area: %s)", id.c_str(), _area.c_str()));

	static const std::string empty; //make some stupid compilers happy.
	return empty;
}


void II18n::load(const std::string &file, const std::string &language) {
	_lang = language;
	//_strings.clear();
	_unlocalized.clear();
	LOG_DEBUG(("loading file '%s' with language: %s", file.c_str(), language.empty()?"default":language.c_str()));
	
	parseFile(file);
	
	for(std::set<std::string>::const_iterator i = _unlocalized.begin(); i != _unlocalized.end(); ++i) {
		LOG_WARN(("unlocalized message with id %s", i->c_str()));
	}
	_unlocalized.clear();	
}

void II18n::start(const std::string &name, Attrs &attr) {
	if (name == "string") {
		_string_id = attr["id"];
		if (_string_id.empty())
			throw_ex(("area must have id"));
		_string_lang = attr["lang"];
	} else if (name == "area") {
		const std::string id = attr["id"];
		if (id.empty())
			throw_ex(("area must have id"));
		_path.push_back(id);
	}
}

void II18n::end(const std::string &name) {
	if (name == "string") {
		std::string path;
		{ //damn this msvc!
		for(size_t i = 0; i < _path.size(); ++i) {
			path += _path[i];
			path += "/";
		}
		}
		path += _string_id;
		Strings::iterator i = _strings.find(path);
		
		if (i == _strings.end()) {
			if (_string_lang.empty() || _lang == _string_lang)
				_strings[path] = _cdata;

			if (_string_lang.empty() && !_lang.empty())
				_unlocalized.insert(path);
			
		} else {
			if (!_string_lang.empty() && _string_lang == _lang) {
				i->second = _cdata;
				_unlocalized.erase(path);
			}
		}
	} else if (name == "area") {
		assert(!_path.empty());
		_path.pop_back();
	}
	_cdata.clear();
}

void II18n::charData(const std::string &data) {
	_cdata += data;
}
