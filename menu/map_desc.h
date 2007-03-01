#ifndef BTANKS_MENU_MAP_DESC_H__
#define BTANKS_MENU_MAP_DESC_H__

#include <string>

	struct MapDesc {
		std::string base, name, desc, object_restriction, game_type;
		int slots;
		MapDesc(const std::string &base, const std::string &name, const std::string &desc, const std::string &object, const std::string &game_type, const int slots) : 
			base(base), name(name), desc(desc), object_restriction(object), game_type(game_type), slots(slots) {
				if (game_type.empty()) 
					this->game_type = "deathmatch";
			}

		const bool operator<(const MapDesc &other) const;
	};


#endif
