#ifndef BTAKS_SHOP_ITEM_H___
#define BTAKS_SHOP_ITEM_H___

#include "container.h"
#include "campaign.h"

class Label;
class Animation;
class AnimationModel;
class Pose;

struct ShopItem : public Container {
public:
	ShopItem(const Campaign &campaign, const Campaign::ShopItem &item, const int w);
	void revalidate(const Campaign &campaign, const Campaign::ShopItem &item, const bool active);
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual void tick(const float dt);

private: 
	Label *_name, *_price, *_amount;
	bool _active;
	const Animation *_animation;
	const AnimationModel *_animation_model;
	const Pose * _pose;
	
	float t;
};

#endif

