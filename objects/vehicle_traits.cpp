#include "vehicle_traits.h"
#include "config.h"
#include "mrt/exception.h"

void VehicleTraits::getWeaponCapacity(int &max_n, int &max_v, const std::string &vehicle, const std::string &object, const std::string &type) {
	if (object.empty()) {
		max_n = 0;
		max_v = 0;
		return;
	}
	
	if (vehicle.empty() || object.empty() || type.empty())
		throw_ex(("vehicle(%s)/object(%s)/type(%s) cannot be empty", vehicle.c_str(), object.c_str(),type.c_str()));
	
	if (object != "missiles" && object != "mines")
		throw_ex(("`weapon` must be missiles or mines."));
	
	const std::string key = "objects." + type + "-" + object + "-on-" + vehicle;
	
	int def_cap = 10;
	int def_v = 1;

	if (vehicle == "launcher") {
		def_v = (type == "nuke")?2:3;
		if (type == "guided") 
			def_cap = 15;
		else if (type == "nuke")
			def_cap = 4;
		else if (type == "stun")
			def_cap = 6;
			
	} else if (vehicle == "tank") {
		if (type == "nuke")
			def_cap = 3;
		else if (type == "boomerang") 
			def_cap = 6;
		else if (type == "dumb") 
			def_cap = 8;
		else if (type == "stun")
			def_cap = 4;
					
	} else if (vehicle == "boat") {
		def_v = (type == "nuke")?2:3;
		def_cap = 5;
	}
	
	Config->get(key + ".capacity", max_n, def_cap);

	Config->get(key + ".visible-amount", max_v, def_v);
}
