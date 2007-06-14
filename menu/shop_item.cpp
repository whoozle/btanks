#include "shop_item.h"
#include "label.h"
#include "i18n.h"
#include "resource_manager.h"
#include "animation_model.h"

ShopItem::ShopItem(const Campaign &campaign, const Campaign::ShopItem &item, const int w) : _active(false), t(0) {
	add(0, 0, _name = new Label("medium", item.name));
	add(w / 2, 0, _price = new Label("medium", mrt::formatString("%d", item.price).c_str()));
	add(3 * w / 4, 0, _amount = new Label("medium", "0"));
	revalidate(campaign, item, false);
}

void ShopItem::revalidate(const Campaign &campaign, const Campaign::ShopItem &item, const bool active) {
	_active = active;
	int cash = campaign.getCash();
	std::string font = item.price > cash?"medium_dark":"medium";
	_name->setFont(font);
	_price->setFont(font);
	_amount->setFont(font);
	_amount->set(mrt::formatString("%d", item.amount));
	
	if (!item.object.empty() && !item.animation.empty() && !item.pose.empty()) {
		_animation = ResourceManager.get_const()->getAnimation(item.animation);
		_animation_model = ResourceManager->getAnimationModel(_animation->model);
		_pose = _animation_model->getPose(item.pose);
	} else {
		_animation = NULL; 
		_animation_model = NULL;
		_pose = NULL;
	}
}

void ShopItem::render(sdlx::Surface &surface, const int x, const int y) {
	Container::render(surface, x, y);
	if (!_active || _pose == NULL || _animation == NULL || _animation_model == NULL) 
		return;
	int w, h; 
	getSize(w, h);
}

void ShopItem::tick(const float dt) {
	Container::tick(dt);
	t += dt;
	t -= (int)t;
}
