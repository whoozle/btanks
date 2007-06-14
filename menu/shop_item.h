#ifndef BTAKS_SHOP_ITEM_H___
#define BTAKS_SHOP_ITEM_H___

#include "container.h"

struct ShopItem : public Container {
	std::string type, name;
	int price, max;
};

#endif

