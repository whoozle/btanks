#ifndef BTANKS_SIMPLE_JOY_BINDINGS
#define BTANKS_SIMPLE_JOY_BINDINGS

#include <string>

class SimpleJoyBindings {
public: 
	struct State {
		enum Type {None, Axis, Button, Hat} type;
		int index, value;
		State() : type(None), index(-1), value(0) {}
		State(Type type, int index, int value = 0) : type(type), index(index), value(value) {}
		
		const std::string to_string() const;
		void from_string(const std::string &value);
	};

	//by index (0-8) get joystick state (Axis, 1, -1 (negative)), (Button, 2), (Hat, Center)
	SimpleJoyBindings(const std::string &profile, int axis, int buttons, int hats);
	void save();
	void reload();
private: 
	std::string config_base;
	int axis, buttons, hats;
	
	State state[8];
};

#endif
