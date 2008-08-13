
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
#include "i18n.h"
#include "mrt/exception.h"
#include <assert.h>
#include <string.h>
#include "finder.h"
#include "mrt/scoped_ptr.h"
#include "mrt/base_file.h"

IMPLEMENT_SINGLETON(I18n, II18n);

bool lessnocase::operator()(const std::string& s1, const std::string& s2) const {
#ifdef _WINDOWS
		return _stricmp(s1.c_str(), s2.c_str()) < 0;
#else
		return strcasecmp(s1.c_str(), s2.c_str()) < 0;
#endif
}

void II18n::load(const std::string &lang) {
	IFinder::FindResult strings_files;
	Finder->findAll(strings_files, "strings.xml");
	for(size_t i = 0; i < strings_files.size(); ++i) 
		load(strings_files[i].second, lang);
}

II18n::II18n() {
	_langs.insert("en");
}

void II18n::enumerateKeys(std::deque<std::string> &keys, const std::string &area) const {
	std::string base = area;

	keys.clear();
	
	for(Strings::const_iterator i = _strings.begin(); i != _strings.end(); ++i) {
		if (!base.empty() && i->first.compare(0, base.size(), base) != 0)
			continue;
		keys.push_back(i->first.substr(base.size()));
	}
}


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

const std::string& II18n::get(const std::string &id) const {
	if (id.empty())
		throw_ex(("I18n->get(/empty-id/) is not allowed"));

	Strings::const_iterator i = _strings.find(id);
	if (i == _strings.end())
		throw_ex(("message with id %s could not be found. (raw get)", id.c_str()));
	return i->second;
}

bool II18n::has(const std::string &id) const {
	if (id.empty())
		throw_ex(("I18n->has(/empty-id/) is not allowed"));

	return _strings.find(id) != _strings.end();
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
			area.resize(p - 1);
	} while (true);
	

	throw_ex(("message with id %s could not be found. (initial area: %s)", id.c_str(), _area.c_str()));

	static const std::string empty; //make some stupid compilers happy.
	return empty;
}


void II18n::load(const std::string &file, const std::string &language) {
	_lang = language;
	//_strings.clear();
	_unlocalized.clear();
	_cdata.clear();
	LOG_DEBUG(("loading file '%s' with language: %s", file.c_str(), language.empty()?"default":language.c_str()));

	scoped_ptr<mrt::BaseFile> f ( Finder->get_file(file, "rt") );
	parse_file(*f);
	f->close();
	
	for(std::set<std::string>::const_iterator i = _unlocalized.begin(); i != _unlocalized.end(); ++i) {
		LOG_WARN(("unlocalized message \"%s\"", i->c_str()));
	}
	_unlocalized.clear();	
}

void II18n::start(const std::string &name, Attrs &attr) {
	_cdata.clear();
	if (name == "string") {
		_string_id = attr["id"];
		if (_string_id.empty())
			throw_ex(("area must have id"));
		_string_lang = attr["lang"];
		if (!_string_lang.empty())
			_langs.insert(_string_lang);
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

void II18n::cdata(const std::string &data) {
	_cdata += data;
}
