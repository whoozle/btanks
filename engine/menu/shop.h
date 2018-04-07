#ifndef BTANKS_SHOP_H__
#define BTANKS_SHOP_H__

#include "container.h"
#include <string>

class ScrollList;
class Campaign;

class Shop : public Container {
public: 
	Shop(const int w, const int h);	
	void init(Campaign *campaign);
	virtual bool onKey(const SDL_Keysym sym);
	virtual void tick(const float dt);

	void revalidate();
private: 
	Campaign *_campaign;
	std::string _prefix;
	ScrollList *_wares;
};


#endif

