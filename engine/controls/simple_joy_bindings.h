#ifndef BTANKS_SIMPLE_JOY_BINDINGS
#define BTANKS_SIMPLE_JOY_BINDINGS

#include <string>

namespace sdlx {
	class Joystick;
}

class SimpleJoyBindings {
public: 
	struct State {
		enum Type {None, Axis, Button, Hat} type;
		int index, value;
		bool need_save;
		
		inline State() : type(None), index(-1), value(0), need_save(false) {}
		inline State(Type type, int index, int value) : type(type), index(index), value(value), need_save(false) {}
		
		const std::string get_name() const;
		const std::string to_string() const;
		void from_string(const std::string &value);
		inline void clear() {
			type = None; index = -1; value = 0; need_save = false;
		}
		inline bool operator<(const State &o) const {
			if (type != o.type)
				return type < o.type;
			if (index != o.index)
				return index < o.index;

			return value < o.value;
		}
		inline bool operator==(const State &o) const {
			return type == o.type && index == o.index && value == o.value;
		}
	};

	//by index (0-8) get joystick state (Axis, 1, -1 (negative)), (Button, 2), (Hat, Center)
	SimpleJoyBindings() : config_base(), axis(0), buttons(0), hats(0) {}
	SimpleJoyBindings(const std::string &profile, const sdlx::Joystick &joy);
	void save();
	void reload();
	void set(int idx, const State &state);
	bool valid() const;
	
	const std::string get_name(int idx) const;

private: 
	void validate();
	static void set_opposite(State &dst, const State &src);
	
	State state[8];
	std::string config_base;
	int axis, buttons, hats;
	
};

#endif
