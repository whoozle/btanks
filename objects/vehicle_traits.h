#ifndef __BTANKS_VEHICLE_TRAITS_H__
#define __BTANKS_VEHICLE_TRAITS_H__

#include <string>

class VehicleTraits {
public:
static void getWeaponCapacity(int &max_n, int &max_v, 
	const std::string &vehicle, const std::string &object, const std::string &type);

};

#endif

