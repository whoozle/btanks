#ifndef BTANKS_AI_TARGETS_H__
#define BTANKS_AI_TARGETS_H__

#include "mrt/singleton.h"
#include "export_btanks.h"
#include <set>
#include <string>

#define DECLARE_TARGET_LIST(x) std::set<std::string> x

namespace ai {

class BTANKSAPI ITargets {
public: 
	DECLARE_SINGLETON(ITargets);
	ITargets();

	DECLARE_TARGET_LIST(troops);
	DECLARE_TARGET_LIST(troops_and_train);
	DECLARE_TARGET_LIST(troops_and_missiles);
	DECLARE_TARGET_LIST(troops_train_and_missiles);
	DECLARE_TARGET_LIST(players);
	DECLARE_TARGET_LIST(players_and_monsters);
	DECLARE_TARGET_LIST(monster);
	DECLARE_TARGET_LIST(infantry); //minimal: kamikaze, player, troops
	DECLARE_TARGET_LIST(infantry_and_train); //minimal: kamikaze, player, troops

private: 
	void insert(std::set<std::string> &x, const char *array[]);
};

PUBLIC_SINGLETON(BTANKSAPI, Targets, ITargets);

}


#endif

