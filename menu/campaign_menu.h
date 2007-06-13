#ifndef BTANKS_CAMPAIGN_MENU_H__
#define BTANKS_CAMPAIGN_MENU_H__

#include "menu/base_menu.h"
#include "mrt/xml.h"
#include "sdlx/rect.h"

class MainMenu;
class Chooser;
class ScrollList;
class Label;
class Button;
class Shop;
class ImageView;

namespace sdlx {
	class sdlx::Surface;
}

struct Campaign : protected mrt::XMLParser {
	Campaign();
	std::string base, name, title;
	int minimal_score;
	
	const sdlx::Surface *map;
	
	struct Map {
		std::string id;
		std::string visible_if;
		v2<int> position;
	};
	
	std::vector<Map> maps;
	
	void init();
	const bool visible(const Map &map_id) const;
	
protected: 
	void getStatus(const std::string &map_id, bool &played, bool &won) const;

	void start(const std::string &name, Attrs &attr);
	void end(const std::string &name);
};



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

	sdlx::Rect map_view;
	ImageView * _map_view;
};

#endif
