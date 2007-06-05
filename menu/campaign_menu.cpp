#include "campaign_menu.h"
#include "box.h"
#include "finder.h"

CampaignMenu::CampaignMenu(MainMenu *parent, const int w, const int h) : _parent(parent) {
	IFinder::FindResult files;

	Finder->findAll(files, "campaign.xml");
	if (files.empty())
		return;

	LOG_DEBUG(("found %u campaign(s)", (unsigned)files.size()));

	Box *b = new Box("menu/background_box.png", w - 32, h - 32);
	int bw, bh;
	b->getSize(bw, bh);
	add((w - bw) / 2, (h - bh) / 2, b);
}

const bool CampaignMenu::empty() const {
	return _campaigns.empty();
}

