#ifndef BTANKS_SPECIAL_ZONE_H__
#define BTANKS_SPECIAL_ZONE_H__

#include "zbox.h"
#include <string>

class SpecialZone : public ZBox {
public: 
	std::string type, name, area;

	SpecialZone(const ZBox & zbox, const std::string &type, const std::string &name);

	void onEnter(const int slot_id);

	const bool final() const;
private: 
	void onCheckpoint(const int slot_id);
	void onHint(const int slot_id);
};

#endif

