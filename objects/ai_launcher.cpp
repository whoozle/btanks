/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "launcher.h"
#include "mrt/exception.h"
#include "resource_manager.h"
#include "config.h"

#include "fann/network.h"

class AILauncher : public Launcher {
public: 
	AILauncher();

	virtual void emit(const std::string &event, Object * emitter);
	virtual void calculate(const float dt);
	virtual void onSpawn();
	virtual Object * clone() const;
	virtual void serialize(mrt::Serializator &s) const {
		Launcher::serialize(s);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Launcher::deserialize(s);
	}	
private: 
	fanncxx::Network _network;
};


void AILauncher::calculate(const float dt) {

}

void AILauncher::onSpawn() {
	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");
	bool loaded = false;
	TRY {
		_network.load(mrt::formatString("%s/neural/%s", data_dir.c_str(), registered_name.c_str()));
		loaded = true;
	} CATCH("loading network", {});
	
	if (!loaded) {
		LOG_WARN(("could not load predefined network for %s", registered_name.c_str())); 

		int layers;
		Config->get("neural." + registered_name + ".layers", layers, 5);

		std::string config;
		Config->get("neural." + registered_name + ".layers-configuration", config, "10, 10, 10, 6");
		std::vector<std::string> res;
		mrt::split(res, config, ",");

		unsigned int *nums = NULL;
		nums = new unsigned int[layers];
		
		int i, n = res.size();
		for(i = 0; i < n; ++i) {
			if (i > layers)
				throw_ex(("layers-configuration contains values more than layers count"));
			int x = atoi(res[i].c_str());
			if (x <= 0)
				throw_ex(("layers-configuration[%d] must not contain negative/zero values", i));
				
			nums[i] = x;
		}
		if (i < layers)
			throw_ex(("layers-configuration does not contain enough data for initialization"));
		
		TRY {
			_network.create(fanncxx::Network::Standard, layers, nums);
		} CATCH("create network", { delete[] nums; throw; });
		delete[] nums;
	}
	assert(!_network.isNull());
	Launcher::onSpawn();
}

void AILauncher::emit(const std::string &event, Object * emitter) {
	Launcher::emit(event, emitter);
}


AILauncher::AILauncher() {}

Object * AILauncher::clone() const {
	return new AILauncher(*this);
}

REGISTER_OBJECT("ai-launcher", AILauncher, ());
