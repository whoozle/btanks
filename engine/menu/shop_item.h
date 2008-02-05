#ifndef BTAKS_SHOP_ITEM_H___
#define BTAKS_SHOP_ITEM_H___

#include "container.h"
#include "campaign.h"

class Label;
class Animation;
class AnimationModel;
class Pose;
class Button;

struct ShopItem : public Container {
public:
	ShopItem(const Campaign &campaign, const Campaign::ShopItem &item, const int w);
	void revalidate(const Campaign &campaign, const Campaign::ShopItem &item, const bool active);
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual void tick(const float dt);
	
	const bool wasSold() const { return sold; }

private: 
	Label *_name, *_price, *_amount;
	Button * _b_plus, *_b_minus;
	bool _active;
	const Animation *_animation;
	const AnimationModel *_animation_model;
	const sdlx::Surface *_surface;
	const Pose * _pose;
	
	int xbase, ybase;
	
	float t, dir_speed, dir_t;
	bool sold;
};

#endif

