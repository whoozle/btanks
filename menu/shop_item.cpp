#include "shop_item.h"
#include "label.h"
#include "i18n.h"

ShopItem::ShopItem(const Campaign &campaign, const Campaign::ShopItem &item, const int w) {
	add(0, 0, new Label("medium", item.name));
}
