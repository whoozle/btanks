#include "host_item.h"
#include "label.h"
#include "resource_manager.h"

HostItem::HostItem() : ping(0), players(0), slots(0) , _line(new Label("small", "")) {
	add(0, 0, _line);
}

void HostItem::update() {
	std::string prefix = slots != 0? mrt::formatString("[%d/%d] ", players, slots) : std::string("[-/-] ");
	std::string pingstr = ping > 0? mrt::formatString(", ping: %d ms)", ping - 1) : std::string();
	std::string hoststr = name;
	if (hoststr.empty()) {
		hoststr = ip;
	} else if (!ip.empty()) {
		hoststr += " (" + ip + ")";
	}
	_line->set(prefix + hoststr + pingstr);
}
