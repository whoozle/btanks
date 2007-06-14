#include "shop_item.h"
#include "label.h"
#include "i18n.h"

ShopItem::ShopItem(const Campaign &campaign, const Campaign::ShopItem &item, const int w) {
	add(0, 0, _name = new Label("medium", item.name));
	add(w / 2, 0, _price = new Label("medium", mrt::formatString("%d", item.price).c_str()));
	add(3 * w / 4, 0, _amount = new Label("medium", "0"));
	revalidate(campaign, item);
}

void ShopItem::revalidate(const Campaign &campaign, const Campaign::ShopItem &item) {
	int cash = campaign.getCash();
	std::string font = item.price > cash?"medium_dark":"medium";
	_name->setFont(font);
	_price->setFont(font);
	_amount->setFont(font);
	_amount->set(mrt::formatString("%d", campaign.getAmount(item)));
}

