#ifndef __BTANKS_PROXY_ITEM_H__
#define __BTANKS_PROXY_ITEM_H__

#include "menuitem.h"
class MainMenu;

class ProxyItem : public MenuItem {
public:
	ProxyItem(const MainMenu &parent, sdlx::Font &font, const std::string &name, const std::string &type, const std::string &text, 
		const std::string &menu, const std::string &itemname);
	virtual const std::string getValue() const;
private: 
	const MainMenu &_parent;
	std::string _menu, _name;
};
#endif
