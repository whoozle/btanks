#include "map_desc.h"

const bool MapDesc::operator<(const MapDesc & other) const {
	if (base != other.base)
		return base < other.base;
	return name < other.name;	
} 
