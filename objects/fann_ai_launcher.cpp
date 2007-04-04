#if 0
/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
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

#include "math/matrix.h"
#include "math/unary.h"

#include "tmx/map.h"

#include "mrt/random.h"

#include "fann/network.h"

class AILauncher : public Launcher {
public: 
	AILauncher();
	~AILauncher();

	virtual void emit(const std::string &event, Object * emitter);
	virtual void calculate(const float dt);
	virtual void onSpawn();
	virtual Object * clone() const;
	virtual void serialize(mrt::Serializator &s) const {
		Launcher::serialize(s);
		s.add(_reaction);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Launcher::deserialize(s);
		s.get(_reaction);
	}	
private: 
	static void movementTraining(float *output, const Matrix<int> &surrounds);

	Alarm _reaction;
	//v2<float> _old_position;
	fanncxx::Network _network;
};

#define OLEFT 0
#define ORIGHT 1
#define OUP 2
#define ODOWN 3
#define OFIRE 4
#define OFIRE2 5

void AILauncher::movementTraining(float *output, const Matrix<int> &surr) {
	LOG_DEBUG(("training movement..."));
	GET_CONFIG_VALUE("objects.ai-launcher.visible-surrounds-matrix-size", int, vss, 5);
	int mc = (vss - 1)/2; //matrix center, 3x3 - 1, 5x5 - 2, 7x7 - 3
	float v = 0, h = 0;
	float v_abs = 0, h_abs = 0;
	for(int circle = 1; circle <= mc; ++circle) {
		int size = circle * 2 + 1; //3, 5, 7
		int p = mc - circle;

		//test horisontals
		int r = circle * circle;
		int y = p, x;
		{
			float vw = 0;
			for(x = 0; x < size; ++x) {
				vw -= 1.0 - surr.get(y, p + x) / 100.0;
			}
			v_abs -= vw / r;
			y = vss - p - 1;
			for(x = 0; x < size; ++x) {
				vw += 1.0 - surr.get(y, p + x) / 100.0;
			}
			v_abs += vw / r;
			v += vw / r;
		}
		//test verticals
		x = p;
		{
			float hw = 0;
			for(y = 0; y < size; ++y) {
				hw -= 1.0 - surr.get(p + y, x) / 100.0;
			}
			h_abs -= hw / r;
			x = vss - p - 1;
			for(y = 0; y < size; ++y) {
				hw += 1.0 - surr.get(p + y, x) / 100.0;
			}
		
			h_abs += hw / r;
			h += hw / r;
		}
	}
	LOG_DEBUG(("h = %g, v = %g, [h] = %g, [v] = %g", h, v, h_abs, v_abs));
	
	//testing results.
	/*
	float t = 0.3;
	output[OLEFT]  = (h < -t)?1:0;
	output[ORIGHT] = (h > t)?1:0;
	output[OUP]    = (v < -t)?1:0;
	output[ODOWN]  = (v > t)?1:0;
	*/
	
/*	if (math::abs(h) > 1.0)
		h = math::sign(h);
	if (math::abs(v) > 1.0)
		v = math::sign(h);
*/	
	if (math::abs(h) < 0.01) {
		if (h_abs > 1) {
			output[OLEFT] = (h < 0)?1:0;
			output[ORIGHT] = (h >= 0)?1:0;
		} else {
			output[OLEFT] = output[ORIGHT] = 0;
		}
	} else {
		if (math::sign(h) > 0) {
			output[ORIGHT] = 1;
			output[OLEFT] = 0;
		} else {
			output[ORIGHT] = 0;
			output[OLEFT] = 1;		
		}
	}

	if (math::abs(v) < 0.01) {
		if (v_abs > 1) {
			//random movement, cannot choose position.
			output[OUP] = (v < 0)?1:0;
			output[ODOWN] = (v >= 0)?1:0;
		} else {
			output[OUP] = output[ODOWN] = 0;
		}
	} else {
		if (math::sign(v) > 0) {
			output[ODOWN] = 1;
			output[OUP] = 0;
		} else {
			output[ODOWN] = 0;
			output[OUP] = 1;
		}
	}

}


static inline void state2output(const PlayerState &state, float *output) {
	output[OLEFT] = state.left?1:0;
	output[OUP] = state.up?1:0;
	output[ORIGHT] = state.right?1:0;
	output[ODOWN] = state.down?1:0;
	output[OFIRE] = state.fire?1:0;
	output[OFIRE2] = state.alt_fire?1:0;
}

static inline void output2state(const float *output, PlayerState &state) {
	state.left = output[OLEFT] >= 0.5;
	state.up   = output[OUP] >= 0.5;
	state.right= output[ORIGHT] >= 0.5;
	state.down = output[ODOWN] >= 0.5;
	state.fire = output[OFIRE] >= 0.5;
	state.alt_fire = output[OFIRE2] >= 0.5;
}


void AILauncher::calculate(const float dt) {
	if (!_reaction.tick(dt))
		return;
	bool train_movement = false;
	if (_velocity.is0())
		train_movement = true;
	
	//getPosition(_old_position);
	GET_CONFIG_VALUE("objects.ai-launcher.visible-surrounds-matrix-size", int, vss, 5);
	Matrix<int> m(vss, vss, 100);
	
	v2<int> pos;
	getCenterPosition(pos);
	Map->getSurroundings(m, pos, 100);
	
	int mc = (vss - 1) / 2;
	float *input = NULL;
	int input_size = vss * vss - 1;
	LOG_DEBUG(("surrounds = %s", m.dump().c_str()));
	TRY { 
		//preparing input
		input = new float[input_size];
		int p = 0;
		for(int y = 0; y < vss; ++y) 
			for(int x = 0; x < vss; ++x) {
				if (x == mc && y == mc)
					continue;
				
				input[p++] = m.get(y, x) / 100.0;
			}


		//running network
		float *output = _network.run(input);

		//dumping in and out
		std::string r;
		for(int i = 0; i < p; ++i) 
			r += mrt::formatString("%0.2f%s", input[i], i < p - 1?", ":"");
		LOG_DEBUG(("input = [%s]", r.c_str()));
		r.clear();
		for(int i = 0; i < 6; ++i) 
			r += mrt::formatString("%0.2f%s", output[i], i < 5?", ":"");

		LOG_DEBUG(("output = [%s]", r.c_str()));

		//analyzing result
		output2state(output, _state);
		LOG_DEBUG(("output = %s", _state.dump().c_str()));
		
		if ((_state.left && _state.right) || 
			(_state.up && _state.down) || 
			(!_state.left && !_state.right && !_state.up && !_state.down))
				train_movement = true;
						
		if (train_movement) {
			movementTraining(output, m);
			output2state(output, _state);
			LOG_DEBUG(("training state: %s", _state.dump().c_str()));
			r.clear();
			for(int i = 0; i < 6; ++i) 
				r += mrt::formatString("%0.2f%s", output[i], i < 5?", ":"");

			LOG_DEBUG(("training output = [%s]", r.c_str()));
			_network.train(input, output);
			//LOG_DEBUG(("MSE = %g", _network.getMSE()));
			//_network.printConnections();
		}
		
		_state.fire = _state.alt_fire = false;
	} CATCH("run", { delete []input; input = NULL;});
	
	Launcher::calculate(dt);
	//LOG_DEBUG(("surrounds: %s", m.dump().c_str()));
}

void AILauncher::onSpawn() {
	GET_CONFIG_VALUE("objects.ai-launcher.reaction-time", float, rt, 0.1);
	_reaction.set(rt);

	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");
	bool loaded = false;
	TRY {
		_network.load(mrt::formatString("%s/neural/%s", data_dir.c_str(), registered_name.c_str()));
		loaded = true;
	} CATCH("loading network", {});
	
	if (!loaded) {
		LOG_WARN(("could not load predefined network for %s", registered_name.c_str())); 

		std::string config;
		Config->get("objects." + registered_name + ".layers-configuration", config, "10,10");
		std::vector<std::string> res;
		mrt::split(res, config, ",");

		unsigned int *nums = NULL;
		
		int i, n = res.size();
		nums = new unsigned int[n + 2];

		GET_CONFIG_VALUE("objects.ai-launcher.visible-surrounds-matrix-size", int, vss, 5);
		nums[0] = vss * vss - 1;
		
		for(i = 0; i < n; ++i) {
			int x = atoi(res[i].c_str());
			if (x <= 0)
				throw_ex(("layers-configuration[%d] must not contain negative/zero values", i));
				
			nums[i + 1] = x;
		}
		nums[n + 1] = 6;
		
		TRY {
			_network.create(fanncxx::Network::Standard, n + 2, nums);
			//_network.randomizeWeights(-0.5, 0.5);
			//_network.setLearningRate(0.99);
			//_network.setTrainingAlgo(FANN_TRAIN_QUICKPROP);
			//_network.setTrainingAlgo(FANN_TRAIN_INCREMENTAL);
			assert(_network.getNumInput() == (unsigned)(vss*vss - 1));
			assert(_network.getNumOutput() == 6);
		} CATCH("create network", { delete[] nums; throw; });
		delete[] nums;
	}
	assert(!_network.isNull());
	//_network.printConnections();
	Launcher::onSpawn();
}

AILauncher::~AILauncher() {
	if (!_network.isNull()) {
		GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");
		_network.save(mrt::formatString("%s/neural/%s", data_dir.c_str(), registered_name.c_str())); //fixme: remove it.
	}
}

void AILauncher::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
	}
	Launcher::emit(event, emitter);
}


AILauncher::AILauncher() : BaseAI("vehicle"), _reaction(true) {}

Object * AILauncher::clone() const {
	return new AILauncher(*this);
}

REGISTER_OBJECT("ai-launcher", AILauncher, ());

#endif
