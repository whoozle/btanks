#ifndef BTANKS_I18N_H__
#define BTANKS_I18N_H__

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

#include "mrt/xml.h"
#include <map>
#include <set>
#include <deque>
#include <string>


struct lessnocase
{
	bool operator()(const std::string& s1, const std::string& s2) const;
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
