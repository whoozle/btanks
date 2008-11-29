#ifndef BTANKS_CAMPAIGN_MENU_H__
#define BTANKS_CAMPAIGN_MENU_H__

#include "mrt/xml.h"
#include "sdlx/rect.h"
#include "campaign.h"
#include "container.h"

class Box;
class Chooser;
class ScrollList;
class Label;
class Button;
class Shop;
class ImageView;
class Grid;
class Medals;
class Image;

class CampaignMenu : public Container {
public: 
	CampaignMenu(const int w, const int h);
	const bool empty() const;

	virtual void tick(const float dt);
	virtual bool onKey(const SDL_keysym sym);
	
	void start();
	void update_map();
	
private:
	void init();
	static const std::string convert_time(const float t);
	static void update_time(Label *l, const std::string &name);
	static void update_score(Label *l, const std::string &name);

	int _w, _h;
	
	typedef std::vector<Campaign> Compaigns;
	Compaigns _campaigns;
	Chooser *_active_campaign;

	ScrollList *_maps;	
	std::vector<int> map_id;
	
	Grid * score_grid;
	Box * score_box;
	Label * _score, *_last_time, *_best_time, *_last_score, *_best_score;

	bool _invalidate_me;
	Shop *_shop;

	Button * _b_shop, *_b_medals, *_b_start;
	Chooser *_c_difficulty;
	ImageView * _map_view;
	Medals * medals;
	std::vector<Image *> medal_icons;
};

#endif
