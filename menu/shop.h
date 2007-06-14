#ifndef BTANKS_SHOP_H__
#define BTANKS_SHOP_H__

#include "container.h"
#include <string>

class ScrollList;
class Campaign;

class Shop : public Container {
public: 
	Shop(const int w, const int h);	
	void init(const Campaign &campaign);
	virtual bool onKey(const SDL_keysym sym);

	void revalidate();
private: 
	const Campaign *_campaign;
	std::string _prefix;
	ScrollList *_wares;
};


#endif

