#ifndef BTANKS_HOST_LIST_H__
#define BTANKS_HOST_LIST_H__

#include "scroll_list.h"

class HostList : public ScrollList {
public:
	HostList(const std::string &config_key, const int w, const int h);
	virtual void add(const std::string &item);
	void promote(const size_t i);
	~HostList();

private: 
	std::string _config_key;
};

#endif
