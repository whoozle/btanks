#include "main_menu.h"
#include "menu_item.h"
#include "campaign_menu.h"
#include "start_server_menu.h"
#include "join_server_menu.h"
#include "options_menu.h"

#include "resource_manager.h"
#include "sdlx/font.h"
#include "sound/mixer.h"


#include "network_status.h"
#include "player_manager.h"

bool MainMenu::generate_key_events_for_gamepad;

MainMenu::MainMenu(int w, int h) : active(NULL) , _netstat(new NetworkStatusControl) {
	CampaignMenu * cm = new CampaignMenu(w, h);
	if (cm->empty()) {
		delete cm;
		cm = NULL;
	} else {
		add(new MenuItem("big", "menu", "start-campaign"), cm);
	}
	
	add(new MenuItem("big", "menu", "start-game"), new StartServerMenu(w, h));
	add(new MenuItem("big", "menu", "join-game"), new JoinServerMenu(w, h));
	add(new MenuItem("big", "menu", "options"), new OptionsMenu(w, h));
	
	add(new MenuItem("big", "menu", "credits"));
	add(new MenuItem("big", "menu", "quit"));

	int mw, mh;
	get_size(mw, mh);
	
	const sdlx::Font * font = ResourceManager->loadFont("big", true);

	background.init("menu/background_box.png", w + 128, mh + 32, font->get_height() + 2);
	dx = (w - mw) / 2;
	dy = (h - mh) / 2;
}

void MainMenu::add(MenuItem *item, Control *slave) {
	Menu::add(item);
	items.push_back(slave);
}

void MainMenu::tick(const float dt) {
	if (hidden())
		return;

	if (changed()) {
		reset();
		LOG_DEBUG(("changed %d", current_item));
		if (current_item >=0 && current_item < (int)items.size() && (active = items[current_item]) != NULL) {
			active->hide(false);
		} else {
			active = NULL;
			MenuItem * item = dynamic_cast<MenuItem *>(get_current_item());
			if (item != NULL)
				menu_signal.emit(item->get_id());
		}
	}
	if (active != NULL) {
		if (active->hidden()) {
			active = NULL;
			Mixer->playSample(NULL, "menu/return.ogg", false);
		} else 
			active->tick(dt);
	}
}

void MainMenu::render(sdlx::Surface &surface, const int x, const int y) const {
	if (hidden())
		return;
	
	if (active != NULL && !active->hidden())
		active->render(surface, x, y);
	else
		Menu::render(surface, x + dx, y + dy);
		
	if (PlayerManager->is_server_active())
		_netstat->render(surface, 0, 0);
}

bool MainMenu::onKey(const SDL_keysym sym) {
	if (hidden())
		return false;
	
	if (active != NULL && !active->hidden()) 
		return active->onKey(sym);
	else 
		return Menu::onKey(sym);
}

bool MainMenu::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (hidden())
		return false;

	if (_netstat != NULL && PlayerManager->is_server_active() && _netstat->onMouse(button, pressed, x, y)) {
		if (_netstat->changed()) {
			_netstat->reset();
			PlayerManager->disconnect_all();
		}
		return true;
	}
	
	if (active != NULL && !active->hidden())
		return active->onMouse(button, pressed, x, y);
	else 
		return Menu::onMouse(button, pressed, x, y);
}

bool MainMenu::onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel) {
	if (hidden())
		return false;
	
	if (active != NULL && !active->hidden())
		return active->onMouseMotion(state, x, y, xrel, yrel);
	else
		return Menu::onMouseMotion(state, x, y, xrel, yrel);
}

void MainMenu::on_mouse_enter(bool enter) {
	if (hidden())
		return;
	
	if (active != NULL && !active->hidden())
		active->on_mouse_enter(enter);
	else
		Container::on_mouse_enter(enter);
}

MainMenu::~MainMenu() {
	delete _netstat;
	for(size_t i = 0; i < items.size(); ++i) {
		delete items[i];
	}
}

