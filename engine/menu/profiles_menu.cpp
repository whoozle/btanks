#include "profiles_menu.h"
#include "box.h"
#include "scroll_list.h"
#include "config.h"
#include "new_profile_dialog.h"
#include "button.h"
#include "i18n.h"

ProfilesMenu::ProfilesMenu(const int w, const int h) {
	Box * box = new Box("menu/background_box.png", w - 100, h - 100);
	int bw, bh, mx, my;
	box->get_size(bw, bh);
	box->getMargins(mx, my);
	
	int bx = ( w - bw ) / 2;
	int by = ( h - bh ) / 2;
	
	int base_x = bx + 3 * mx;
	int base_y = by + 3 * my;

	add(bx, by, box);
	
	int cw, ch;
	_list = new ScrollList("menu/background_box_dark.png", "small", bw - 2 * base_x, bh - 2 * base_y);
	add(base_x, base_y, _list);
	_list->get_size(cw, ch);
	
	_ok = new Button("medium_dark", I18n->get("menu", "ok"));
	_add = new Button("medium_dark", I18n->get("menu", "add"));
	_remove = new Button("medium_dark", I18n->get("menu", "delete"));
	
	base_y += ch;
	
	int size[3];
	_ok->get_size(cw, ch);
	size[0] = cw + 16;
	_add->get_size(cw, ch);
	size[1] = cw + 16;
	_remove->get_size(cw, ch);
	size[2] = cw;
	
	cw = 0;
	for(int i = 0; i < 3; ++i)
		cw += size[i];
	
	//base_x = (w - cw) / 2;
	base_x += 16;
	base_y = bh / 2 + base_y / 2;

	add(base_x, base_y, _ok);
	add(base_x + size[0], base_y, _add);
	add(base_x + size[0] + size[1], base_y, _remove);

	_new_profile = new NewProfileDialog();
	_new_profile->get_size(cw, ch);
	add((w - cw) / 2, (h - ch) / 2, _new_profile);
	
	init();
}

void ProfilesMenu::init() {
	_list->clear();
	_ids.clear();
	
	std::set<std::string> keys;
	Config->enumerateKeys(keys, "profile.");
	LOG_DEBUG(("found %u profile(s)", (unsigned)keys.size()));
	for(std::set<std::string>::iterator i = keys.begin(); i != keys.end(); ++i) {
		const std::string &key = *i;
		std::vector<std::string> r;
		mrt::split(r, key, ".", 4);
		if (r[2] != "name")
			continue;

		const std::string &id = r[1];
		
		LOG_DEBUG(("profile '%s'", id.c_str()));

		std::string name, config_key = "profile." + id + ".name";
		Config->get(config_key, name, std::string());
		
		_ids.push_back(id);
		_list->append(name);
	}

	_new_profile->hide();
	_remove->hide(_ids.size() < 2);
}

void ProfilesMenu::save() {
	int idx = _list->get();
	const std::string &id = _ids[idx];
	LOG_DEBUG(("current profile: '%s'", id.c_str()));
	Config->set("engine.profile", id);
}

void ProfilesMenu::tick(const float dt) {
	Container::tick(dt);
	
	if (_ok->changed()) {
		_ok->reset();
		_new_profile->hide();
		save();
		hide();
		return;
	}

	if (_add->changed()) {
		_add->reset();
		_new_profile->hide(false);
	}

	if (_remove->changed()) {
		_remove->reset();
		if (_ids.size() <= 1)
			return;
		
		LOG_DEBUG(("removing profile"));
		
		const std::string &id = _ids[_list->get()];
		Config->remove("profile." + id + ".name");
		Config->remove("profile." + id + ".name-2");
		//do not remove all campaign stuff for now
		
		init();
	}
	
	if (_new_profile->changed()) {
		_new_profile->hide();
		_new_profile->reset();
		const std::string &name = _new_profile->get();
		if (!name.empty()) {
			LOG_DEBUG(("creating new profile"));
			int i;
			std::string key;
			for(i = 0; i < 100; ++i) {
				key = mrt::format_string("profile.%d.name", i);
				if (!Config->has(key))
					break;
				}
			if (i >= 100)
				return;

			Config->set(key, name);

			init();
		}
	}
}

bool ProfilesMenu::onKey(const SDL_keysym sym) {
	if (Container::onKey(sym)) 
		return true;

	switch(sym.sym) {

	case SDLK_KP_ENTER:
	case SDLK_RETURN:
	case SDLK_ESCAPE: 
		save();
		hide();
		return true;

	default: ;
	}
	return false;
}
