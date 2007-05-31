
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
#include "host_list.h"
#include "config.h"
#include "label.h"

HostList::HostList(const std::string &config_key, const int w, const int h) : 
ScrollList("menu/background_box.png", "medium_dark", w, h), _config_key(config_key) {
	std::string str_hosts;
	Config->get(config_key, str_hosts, std::string());
	std::vector<std::string> hosts;
	mrt::split(hosts, str_hosts, " ");
	for(size_t i = 0; i < hosts.size(); ++i) {
		if (hosts[i].empty())
			continue;
		
		mrt::toLower(hosts[i]);
		ScrollList::append(hosts[i]);
	}
}

void HostList::promote(const size_t i) {
	if (i >= _list.size())
		throw_ex(("promote(%u) is out of range", (unsigned)i));
	
	List::iterator li = _list.begin();
	
	for(size_t n = i; n--; ++li);
	Control * host = *li;
	_list.erase(li);	
	_list.push_front(host);
	_current_item = 0;
}

void HostList::append(const std::string &_item) {
	std::string item = _item;
	mrt::toLower(item);
	
	for(List::iterator i = _list.begin(); i != _list.end(); ++i) {
		Label *l = dynamic_cast<Label *>(*i);
		if (l == NULL || l->get().empty()) 
			continue;
		
		if (item == l->get())
			return;
	}
	
	_list.push_front(new Label(_font, item));
}

HostList::~HostList() {
	std::vector<std::string> hosts;
	
	//change it .
	
	for(List::iterator i = _list.begin(); i != _list.end(); ++i) {
		Label *l = dynamic_cast<Label *>(*i);
		if (l == NULL || l->get().empty()) 
			continue;
		hosts.push_back(l->get());
	}
	std::string str;
	mrt::join(str, hosts, " ");
	Config->set(_config_key, str);
}
