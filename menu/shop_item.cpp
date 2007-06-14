#include "shop_item.h"
#include "label.h"
#include "i18n.h"
#include "resource_manager.h"
#include "animation_model.h"
#include "sdlx/surface.h"

ShopItem::ShopItem(const Campaign &campaign, const Campaign::ShopItem &item, const int w) : _active(false), t(0) {
	add(0, 0, _name = new Label("medium", item.name));
	add(w / 2, 0, _price = new Label("medium", mrt::formatString("%d", item.price).c_str()));
	add(3 * w / 4, 0, _amount = new Label("medium", "0"));
	xbase = 7 * w / 16;
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
		_surface = ResourceManager->loadSurface(_animation->surface);
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
	if (_pose == NULL || _animation == NULL || _animation_model == NULL) 
		return;
	int w, h; 
	getSize(w, h);
	int frame = ((int)(t * _pose->speed)) % _pose->frames.size();
	int ybase = h / 2 - _animation->th / 2;
	
	sdlx::Rect from(0, _pose->frames[frame] * _animation->th, _animation->tw, _animation->th);
	surface.copyFrom(*_surface, from, x + xbase - _animation->tw / 2, y + ybase);
}

void ShopItem::tick(const float dt) {
	Container::tick(dt);
	if (_pose == NULL || !_active)
		return;
	t += dt;
	if ((t * _pose->speed) > (int)_pose->frames.size())
		t -= _pose->speed * _pose->frames.size();
}
