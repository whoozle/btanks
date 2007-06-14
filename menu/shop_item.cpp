#include "shop_item.h"
#include "label.h"
#include "i18n.h"

ShopItem::ShopItem(const Campaign &campaign, const Campaign::ShopItem &item, const int w) {
	add(0, 0, _name = new Label("medium", item.name));
	add(w / 2, 0, _price = new Label("medium", mrt::formatString("%d", item.price).c_str()));
	validate(campaign, item);
}

void ShopItem::validate(const Campaign &campaign, const Campaign::ShopItem &item) {
	int cash = campaign.getCash();
	std::string font = item.price > cash?"medium_dark":"medium";
	_name->setFont(font);
	_price->setFont(font);
}

