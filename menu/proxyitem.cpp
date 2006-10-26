#include "proxyitem.h"
#include "menu.h"

ProxyItem::ProxyItem(const MainMenu& parent, sdlx::TTF &font, const std::string &name, const std::string &type, const std::string &text, 
	const std::string &menu, const std::string &itemname
) : 
	MenuItem(font, name, type, text, "**bug**") , _parent(parent), _menu(menu), _name(itemname)
{}

const std::string ProxyItem::getValue() const {
	return _parent.getValue(_menu, _name);
}
