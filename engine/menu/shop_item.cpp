#include "shop_item.h"
#include "label.h"
#include "i18n.h"
#include "resource_manager.h"
#include "animation_model.h"
#include "sdlx/surface.h"
#include "button.h"
#include "math/binary.h"

ShopItem::ShopItem(const Campaign &campaign, const Campaign::ShopItem &item, const int w) : _active(false), t(0), dir_t(0) {
	_name = new Label("medium", item.name);
	int fw, fh;
	_name->get_size(fw, fh);

	int bw, bh;
	_b_plus = new Button("medium", "+");
	_b_plus->get_size(bw, bh);
	_b_minus = new Button("medium", "-");
	
	int h = math::max(bh, fh);
	ybase = h / 2;
	int yfont = h / 2 - fh / 2;

	add(0, yfont, _name);
	add(w / 2, yfont, _price = new Label("medium", mrt::format_string("%d", item.price).c_str()));

	int x_am = 3 * w / 4;
	add(x_am, yfont, _amount = new Label("medium", "0"));
	
	xbase = 7 * w / 16;
	dir_speed = item.dir_speed;

	add(x_am - 112 + bw, h / 2 - bh / 2, _b_minus);
	add(x_am + 32, h / 2 - bh / 2, _b_plus);
	
	revalidate(campaign, item, false);
}

void ShopItem::revalidate(const Campaign &campaign, const Campaign::ShopItem &item, const bool active) {
	_active = active;

	_b_plus->hide(!active);
	_b_minus->hide(!active);
	
	int cash = campaign.getCash();
	std::string font = item.price > cash?"medium_dark":"medium";
	_name->setFont(font);
	_price->setFont(font);
	_amount->setFont(font);
	_amount->set(mrt::format_string("%d", item.amount));
	
	if (!item.object.empty() && !item.animation.empty() && !item.pose.empty()) {
		_animation = ResourceManager.get_const()->getAnimation(item.animation);
		_surface = ResourceManager->load_surface(_animation->surface);
		_animation_model = ResourceManager->get_animation_model(_animation->model);
		_pose = _animation_model->getPose(item.pose);
	} else {
		_animation = NULL; 
		_animation_model = NULL;
		_pose = NULL;
	}
}

void ShopItem::render(sdlx::Surface &surface, const int x, const int y) const {
	Container::render(surface, x, y);
	if (_pose == NULL || _animation == NULL || _animation_model == NULL) 
		return;
	int frame = ((int)(t * _pose->speed)) % _pose->frames.size();

	int dirs = (_surface->get_width() - 1) / _animation->tw + 1;
	int dir = ((int)(dir_t * dir_speed)) % dirs;

	sdlx::Rect from(dir * _animation->tw, _pose->frames[frame] * _animation->th, _animation->tw, _animation->th);
	surface.blit(*_surface, from, x + xbase - _animation->tw / 2, y + ybase - _animation->th / 2);
}

void ShopItem::tick(const float dt) {
	Container::tick(dt);
	if (_b_plus->changed()) {
		_b_plus->reset();
		sold = false;
		invalidate(true);
	}

	if (_b_minus->changed()) {
		_b_minus->reset();
		sold = true;
		invalidate(true);
	}
	
	if (_pose == NULL || _animation == NULL || _surface == NULL || !_active)
		return;
	t += dt;
	dir_t += dt;
	//LOG_DEBUG(("t = %g", t));
	if ((t * _pose->speed) > (int)_pose->frames.size())
		t -= (float)_pose->frames.size() / _pose->speed;

	int dirs = (_surface->get_width() - 1) / _animation->tw + 1;
	if ((dir_t * dir_speed) > dirs)
		dir_t -= (float)dirs / dir_speed;
}
