#include "host_item.h"
#include "label.h"
#include "resource_manager.h"

HostItem::HostItem(const int w) : ping(0), players(0), slots(0) , _line(new Label("small", "")) {
	add(0, 0, _line);
}

void HostItem::update() {
	std::string prefix = slots != 0? mrt::formatString("[%d/%d] ", players, slots) : std::string("[-/-] ");
	std::string pingstr = ping > 0? mrt::formatString(" (%3d ms)", ping) : std::string();
	std::string hoststr = ip;
	if (!host.empty()) {
		hoststr += "(" + host + ")";
	}
	_line->set(prefix + hoststr + pingstr);
}
