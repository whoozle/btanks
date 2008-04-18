#ifndef BTANKS_SPECIAL_OWNERS_H__
#define BTANKS_SPECIAL_OWNERS_H__

#define OWNER_MAP (-42)
#define OWNER_COOPERATIVE (-1)

#include "export_btanks.h"

enum TeamID { TeamNone, TeamRed, TeamGreen, TeamBlue, TeamYellow };

class Object;

const char * BTANKSAPI get_team_color(TeamID t);
TeamID BTANKSAPI get_team(const Object *o);

#endif

