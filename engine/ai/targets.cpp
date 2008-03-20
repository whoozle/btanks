#include "targets.h"

using namespace ai;

IMPLEMENT_SINGLETON(Targets, ITargets);

void ITargets::insert(std::set<std::string> &x, const char * array[]) {
	for(const char **t = array; *t != NULL; ++t) { 
		x.insert(*t); 
	}
}

ITargets::ITargets() {
	const char *a1[] = {"fighting-vehicle", NULL};
	insert(players, a1);
	const char *a1_1[] = {"fighting-vehicle", "monster", NULL};
	insert(players_and_monsters, a1_1);

	const char *a2[] = {"cannon", "fighting-vehicle", "trooper", "kamikaze", "boat", "helicopter", "monster", "watchtower", NULL};
	insert(troops, a2);
	const char *a2_1[] = {"cannon", "fighting-vehicle", "trooper", "kamikaze", "boat", "helicopter", "monster", "watchtower", "train", NULL};
	insert(troops_and_train, a2_1);

	const char *a3[] = {"cannon", "fighting-vehicle", "trooper", "kamikaze", "boat", "helicopter", "monster", "watchtower", "missile", NULL};
	insert(troops_and_missiles, a3);
	const char *a3_1[] = {"cannon", "fighting-vehicle", "trooper", "kamikaze", "boat", "helicopter", "monster", "watchtower", "missile", "train", NULL};
	insert(troops_train_and_missiles, a3_1);

	const char *a4[] = {"fighting-vehicle", "trooper", "cannon", "watchtower", "creature", "civilian", NULL};
	insert(monster, a4);

	const char *a5[] = {"fighting-vehicle", "trooper", "monster", "kamikaze", NULL};
	insert(infantry, a5);
	const char *a6[] = {"fighting-vehicle", "trooper", "monster", "kamikaze", "train", NULL};
	insert(infantry_and_train, a6);
}
