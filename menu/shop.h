#ifndef BTANKS_SHOP_H__
#define BTANKS_SHOP_H__

#include "container.h"
#include <string>

class Shop : public Container {
public: 
	Shop(const int w, const int h);	
	void init(const std::string &campaign);
	virtual bool onKey(const SDL_keysym sym);

private: 
	std::string _campaign, _prefix;
	int _cash;
};


#endif

