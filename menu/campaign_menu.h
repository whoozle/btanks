#ifndef BTANKS_CAMPAIGN_MENU_H__
#define BTANKS_CAMPAIGN_MENU_H__

#include "menu/base_menu.h"

class MainMenu;

class CampaignMenu : public BaseMenu {
public: 
	CampaignMenu(MainMenu *parent, const int w, const int h);
	const bool empty() const;
private:
	MainMenu *_parent;
	
	struct Campaign {
		std::string base;
	};

	typedef std::vector<Campaign> Compaigns;
	Compaigns _campaigns;
};

#endif

