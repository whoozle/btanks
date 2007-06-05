#include "campaign_menu.h"
#include "box.h"
#include "finder.h"
#include "i18n.h"

CampaignMenu::CampaignMenu(MainMenu *parent, const int w, const int h) : _parent(parent) {
	IFinder::FindResult files;

	Finder->findAll(files, "campaign.xml");
	if (files.empty())
		return;

	LOG_DEBUG(("found %u campaign(s)", (unsigned)files.size()));
	for(size_t i = 0; i < files.size(); ++i) {
		LOG_DEBUG(("campaign[%u]: %s", (unsigned)i, files[i].first.c_str()));
		Campaign c;
		c.base = files[i].first;
		c.init();
		_campaigns.push_back(c);
	}

	Box *b = new Box("menu/background_box.png", w - 32, h - 32);
	int bw, bh;
	b->getSize(bw, bh);
	add((w - bw) / 2, (h - bh) / 2, b);
}

const bool CampaignMenu::empty() const {
	return _campaigns.empty();
}

void Campaign::init() {
	parseFile(base + "/campaign.xml");
}

void Campaign::start(const std::string &name, Attrs &attr) {
	if (name == "campaign") {
		if (attr["title"].empty())
			throw_ex(("campaign must have title attr"));
		title = I18n->get("campaign", attr["title"]);
	} else if (name == "map") {
		LOG_DEBUG(("map: %s, if-win: %s, if-lost: %s", name.c_str(), attr["if-win"].c_str(), attr["if-lost"].c_str()));
	}
}

void Campaign::end(const std::string &name) {

}
