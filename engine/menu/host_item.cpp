#include "host_item.h"
#include "label.h"
#include "resource_manager.h"
#include "i18n.h"
#include "config.h"

HostItem::HostItem() : ping(0), players(0), slots(0) , _line(new Label("small", "")) {
	add(0, 0, _line);
	Config->get("multiplayer.port", _default_port, 27255);
}

void HostItem::update() {
	std::string prefix = slots != 0? mrt::format_string("[%d/%d] ", players, slots) : std::string("[-/-] ");
	std::string mapstr;
	
	if (ping > 0) {
		mapstr = "[";
		if (!map.empty())
			mapstr += mrt::format_string("%s: %s, ", I18n->get("menu", "map").c_str(), map.c_str());
		mapstr += mrt::format_string("%s: %d ms]", I18n->get("menu", "ping").c_str(), ping - 1);
	}
	
	std::string hoststr = name, ip = addr.getAddr(addr.port != _default_port);
	if (hoststr.empty()) {
		hoststr = ip;
	} else if (!ip.empty()) {
		hoststr += " (" + ip + ") ";
	}
	hoststr += "  ";
	_line->set(prefix + hoststr + mapstr);
}
