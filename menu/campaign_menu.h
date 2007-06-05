#ifndef BTANKS_CAMPAIGN_MENU_H__
#define BTANKS_CAMPAIGN_MENU_H__

#include "menu/base_menu.h"
#include "mrt/xml.h"
#include "sdlx/rect.h"

class MainMenu;
class Chooser;
class ScrollList;

namespace sdlx {
	class sdlx::Surface;
}

struct Campaign : protected mrt::XMLParser {
	Campaign();
	std::string base, title;
	const sdlx::Surface *map;
	
	std::vector<std::string> maps;
	std::vector<v2<int> > maps_pos;
	
	void init();
	
protected: 
	void start(const std::string &name, Attrs &attr);
	void end(const std::string &name);
};



class CampaignMenu : public BaseMenu {
public: 
	CampaignMenu(MainMenu *parent, const int w, const int h);
	const bool empty() const;

	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual void tick(const float dt);
	
private:
	void init();

	MainMenu *_parent;
	int _w, _h;
	
	typedef std::vector<Campaign> Compaigns;
	Compaigns _campaigns;
	Chooser *_active_campaign;
	ScrollList *_maps;

	sdlx::Rect map_view;
	v2<float> map_pos;
	v2<float> map_dst;
};

#endif

