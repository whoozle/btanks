#ifndef BTANKS_CAMPAIGN_MENU_H__
#define BTANKS_CAMPAIGN_MENU_H__

#include "menu/base_menu.h"
#include "mrt/xml.h"

class MainMenu;
class Chooser;

struct Campaign : protected mrt::XMLParser {
	std::string base, title;
	
	void init();
	
protected: 
	void start(const std::string &name, Attrs &attr);
	void end(const std::string &name);
};



class CampaignMenu : public BaseMenu {
public: 
	CampaignMenu(MainMenu *parent, const int w, const int h);
	const bool empty() const;
	
private:
	MainMenu *_parent;
	
	typedef std::vector<Campaign> Compaigns;
	Compaigns _campaigns;
	Chooser *_active_campaign;
};

#endif

