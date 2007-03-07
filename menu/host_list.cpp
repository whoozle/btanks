#include "host_list.h"
#include "config.h"

HostList::HostList(const std::string &config_key, const int w, const int h) : ScrollList("medium_dark", w, h), _config_key(config_key) {
	std::string str_hosts;
	Config->get(config_key, str_hosts, std::string());
	std::vector<std::string> hosts;
	mrt::split(hosts, str_hosts, " ");
	for(size_t i = 0; i < hosts.size(); ++i) {
		mrt::toLower(hosts[i]);
		ScrollList::add(hosts[i]);
	}
}


void HostList::add(const std::string &_item) {
	std::string item = _item;
	mrt::toLower(item);
	
	for(List::iterator i = _list.begin(); i != _list.end(); ++i) 
		if (item == *i)
			return;
	
	_list.push_front(item);
}

HostList::~HostList() {
	std::vector<std::string> hosts;
	
	//change it .
	
	for(List::iterator i = _list.begin(); i != _list.end(); ++i) {
		hosts.push_back(*i);
	}
	std::string str;
	mrt::join(str, hosts, " ");
	Config->set(_config_key, str);
}
