#ifndef BTANKS_CAMPAIGN_MENU_H__
#define BTANKS_CAMPAIGN_MENU_H__

#include "menu/base_menu.h"
#include "mrt/xml.h"
#include "sdlx/rect.h"
#include "campaign.h"

class MainMenu;
class Chooser;
class ScrollList;
class Label;
class Button;
class Shop;
class ImageView;

class CampaignMenu : public BaseMenu {
public: 
	CampaignMenu(MainMenu *parent, const int w, const int h);
	const bool empty() const;

	virtual void tick(const float dt);
	virtual bool onKey(const SDL_keysym sym);
	
	void start();
	
private:
	void init();

	MainMenu *_parent;
	int _w, _h;
	
	typedef std::vector<Campaign> Compaigns;
	Compaigns _campaigns;
	Chooser *_active_campaign;

	ScrollList *_maps;	
	std::vector<int> map_id;
	
	Label * _score;

	bool _invalidate_me;
	Shop *_shop;

	Button * _b_shop;
	Chooser *_c_difficulty;

	sdlx::Rect map_view;
	ImageView * _map_view;
};

#endif
