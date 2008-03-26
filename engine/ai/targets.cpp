#include "targets.h"

using namespace ai;

IMPLEMENT_SINGLETON(ai::Targets, ai::ITargets);

void ITargets::insert(std::set<std::string> &x, const char * array[]) {
	for(const char **t = array; *t != NULL; ++t) { 
		x.insert(*t); 
	}
}

ITargets::ITargets() {
	players.insert("fighting-vehicle");

	players_and_monsters = players;
	players_and_monsters.insert("monster");

	const char *a2[] = {"cannon", "pillbox", "fighting-vehicle", "trooper", "kamikaze", "boat", "helicopter", "monster", "watchtower", NULL};
	insert(troops, a2);

	troops_and_train = troops;
	troops_and_train.insert("train");

	troops_and_missiles = troops;
	troops_and_missiles.insert("missile");

	troops_train_and_missiles = troops_and_train;
	troops_train_and_missiles.insert("missile");
	
	const char *a4[] = {"fighting-vehicle", "trooper", "cannon", "watchtower", "creature", "civilian", NULL};
	insert(monster, a4);

	const char *a5[] = {"fighting-vehicle", "trooper", "monster", "kamikaze", NULL};
	insert(infantry, a5);

	infantry_and_train = infantry;
	infantry_and_train.insert("train");
}
