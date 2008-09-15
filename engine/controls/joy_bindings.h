#ifndef BTANKS_JOYSTICK_BINDINGS_H__
#define BTANKS_JOYSTICK_BINDINGS_H__

#include <string> 
#include <map>

enum JoyControlType {
	tButton = 1, tAxis = 2, tHat = 3, tBall = 4
};


class Bindings {
public: 
	void load(const std::string &profile, const int buttons, const int axes, const int hats);
	void save();
	void clear();
	
	void set(const JoyControlType type, const int hard_id, const int virt_id);
	const int get(const JoyControlType type, const int virt_id) const;
	const bool has(const JoyControlType type, const int hard_id) const;

private: 
	typedef std::map<const std::pair<JoyControlType, int> , int> BaseBindings;

	std::string _profile;
	BaseBindings _bindings;
};

#endif

