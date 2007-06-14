#ifndef BTAKS_SHOP_ITEM_H___
#define BTAKS_SHOP_ITEM_H___

#include "container.h"
#include "campaign.h"

struct ShopItem : public Container {
public:
	ShopItem(const Campaign &campaign, const Campaign::ShopItem &item, const int w);
};

#endif

