#include "campaign_menu.h"
#include "box.h"
#include "finder.h"
#include "i18n.h"
#include "chooser.h"
#include "resource_manager.h"

CampaignMenu::CampaignMenu(MainMenu *parent, const int w, const int h) : _parent(parent) {
	IFinder::FindResult files;

	Finder->findAll(files, "campaign.xml");
	if (files.empty())
		return;

	LOG_DEBUG(("found %u campaign(s)", (unsigned)files.size()));
	std::vector<std::string> titles;
	for(size_t i = 0; i < files.size(); ++i) {
		LOG_DEBUG(("campaign[%u]: %s", (unsigned)i, files[i].first.c_str()));
		Campaign c;
		c.base = files[i].first;
		c.init();
		_campaigns.push_back(c);
		titles.push_back(c.title);
	}

	Box *b = new Box("menu/background_box.png", w - 32, h - 32);
	int bw, bh;
	b->getSize(bw, bh);
	add((w - bw) / 2, (h - bh) / 2, b);
	int mx, my;
	b->getMargins(mx, my);

	int cw, ch;
	_active_campaign = new Chooser("medium", titles);
	_active_campaign->getSize(cw, ch);
	add(w / 2 - cw / 2, my, _active_campaign);
}

const bool CampaignMenu::empty() const {
	return _campaigns.empty();
}

Campaign::Campaign() : map(NULL) {}


void Campaign::init() {
	map = NULL;
	parseFile(base + "/campaign.xml");
}

void Campaign::start(const std::string &name, Attrs &attr) {
	if (name == "campaign") {
		if (attr["title"].empty())
			throw_ex(("campaign must have title attr"));
		title = I18n->get("campaign", attr["title"]);
		if (attr["map"].empty())
			throw_ex(("campaign must have map attr"));
		map = ResourceManager->loadSurface(attr["map"]);
	} else if (name == "map") {
		LOG_DEBUG(("map: %s, if-won: '%s', if-lost: '%s'", name.c_str(), attr["if-won"].c_str(), attr["if-lost"].c_str()));
	}
}

void Campaign::end(const std::string &name) {

}
