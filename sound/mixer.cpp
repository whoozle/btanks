
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


#include "mixer.h"
#include "mrt/file.h"
#include "mrt/exception.h"
#include "mrt/random.h"
#include "mrt/chunk.h"
#include <assert.h>
#include "utils.h"

#include "ogg_stream.h"
#include "al_ex.h"
#include "sample.h"
#include <AL/alut.h>

#include "config.h"
#include "world.h"
#include "object.h"
#include "math/v3.h"
#include "finder.h"

IMPLEMENT_SINGLETON(Mixer, IMixer)

IMixer::IMixer() : _nosound(true), _nomusic(true), _ogg(NULL) {}

void IMixer::init(const bool nosound, const bool nomusic) {
	if (nosound && nomusic) {
		_nosound = _nomusic = true;
		return;
	}
	
	delete _ogg;
	_ogg = NULL;
	
	TRY {
		alutInit(NULL, NULL);
		AL_CHECK(("alutInit"));
	} CATCH("alutInit", {
		LOG_DEBUG(("there was error(s) during initialization, disabling sounds."));
		_nosound = _nomusic = true;
		return;
	})
	
	TRY {
		GET_CONFIG_VALUE("engine.sound.doppler-factor", float, df, 1.0);
		alDopplerFactor(df);
		AL_CHECK(("alDopplerFactor"));
	
		GET_CONFIG_VALUE("engine.sound.doppler-velocity", float, dv, 1000);
		alDopplerVelocity(dv);
	} CATCH("init", {});
	_nosound = nosound;
	_nomusic = nomusic;
}

void IMixer::deinit() {
	LOG_DEBUG(("cleaning up mixer..."));	
	delete _ogg; 
	_ogg = NULL;
	
	if (!_nosound) {
		LOG_DEBUG(("cleaning up sounds..."));
		std::for_each(_sounds.begin(), _sounds.end(), delete_ptr2<Sounds::value_type>());
		_sounds.clear();
	}
	if (!_nosound || !_nomusic) {
		LOG_DEBUG(("shutting down openAL.."));
		alutExit();
	}

	_nosound = true;
	_nomusic = true;
}

IMixer::~IMixer() {}


void IMixer::loadPlaylist(const std::string &file) {
	if (_nomusic) 
		return;
	
	_playlist.clear();
	TRY {
		mrt::File f;
		f.open(file, "rt");
		std::string line;
		while(f.readLine(line)) {
			mrt::trim(line);
			_playlist[line] = false;
		}
		f.close();
	} CATCH("loadPlayList", {});
	LOG_DEBUG(("loaded %d songs in playlist", _playlist.size()));
}

const bool IMixer::play(const std::string &fname, const bool continuous) {
	if (_nomusic) 
		return false;
	
	LOG_DEBUG(("playing %s",fname.c_str()));
	std::string::size_type dp = fname.rfind('.');
	std::string ext = "unknown";
	if (dp != std::string::npos)
		ext = fname.substr(dp + 1);
	
	if (ext != "ogg") {
		LOG_WARN(("cannot play non-ogg files(%s). fixme.", ext.c_str()));
		return false;
	}

	if (_ogg == NULL) 
		_ogg = new OggStream;

	_ogg->open(fname, continuous);
	return true;
}


void IMixer::play() {
	if (_nomusic) 
		return;

	int n = _playlist.size();
	if (n == 0) {
		LOG_WARN(("nothing to play"));
		_nomusic = true;
		return;
	}
	int p = mrt::random(n);
	
	PlayList::iterator i = _playlist.begin();
	while(p--) ++i;
	assert(i != _playlist.end());
	
	const std::string fname = i->first;
	if (!play(fname))
		return;
	i->second = true;
}

void IMixer::loadSample(const std::string &filename) {
	if (_nosound) 
		return;
	LOG_DEBUG(("loading sample %s", filename.c_str()));
	if (_sounds.find(filename) != _sounds.end()) {
		LOG_DEBUG(("already loaded, skipped."));
		return;
	}

	Sample * sample = NULL;
	TRY {
		sample = new Sample;
		OggStream::decode(*sample, Finder->find("sounds/" + filename));
		LOG_DEBUG(("sample %s decoded. rate: %u, size: %u", filename.c_str(), (unsigned)sample->rate, (unsigned)sample->data.getSize()));

		sample->init();
				
		_sounds[filename] = sample;
	} CATCH("loadSample", { delete sample; sample = NULL; });
}

void IMixer::playSample(const Object *o, const std::string &name, const bool loop) {
	if (_nosound || name.empty())
		return;
	const int id = o->getID();
	//LOG_DEBUG(("object: %d requests %s (%s)", id, name.c_str(), loop?"loop":"single"));
	Sounds::const_iterator i = _sounds.find(name);
	if (i == _sounds.end()) {
		LOG_WARN(("sound %s was not loaded. skipped.", name.c_str()));
		return;
	}
	const Sample &sample = *i->second;
	TRY {
		ALuint source;
		alGenSources(1, &source);
		AL_CHECK(("alGenSources"));
		_sources.insert(Sources::value_type(Sources::key_type(id, name), source));

		v2<float> pos, vel;
		o->getInfo(pos, vel);

		GET_CONFIG_VALUE("engine.sound.positioning-divisor", float, k, 200.0);
	
		ALfloat al_pos[] = { pos.x / k, -pos.y / k, 0*o->getZ() / k };
		ALfloat al_vel[] = { vel.x / k, -vel.y / k, 0 };
	
		alSourcei (source, AL_BUFFER,   sample.buffer);
		alSourcef (source, AL_PITCH,    1.0          );
		alSourcef (source, AL_GAIN,     1.0          );
		alSourcefv(source, AL_POSITION, al_pos       );
		alSourcefv(source, AL_VELOCITY, al_vel       );
		alSourcei (source, AL_LOOPING,  loop?AL_TRUE:AL_FALSE );
		alSourcePlay(source);
	} CATCH("playSample", {});
}

void IMixer::updateObjects() {
	if (!_nomusic) {
		if (_ogg != NULL && !_ogg->alive()) {
			delete _ogg;
			_ogg = NULL;
		}
		if (_ogg == NULL) {
			LOG_DEBUG(("big fixme. remove this ugly sound thread restart..."));
			play();
		}
	}

	if (_nosound) 
		return;
		
	for(Sources::iterator j = _sources.begin(); j != _sources.end();) {
	int last_id = -1;
	Object *o = NULL;
	TRY {
		ALuint source = j->second;
		ALenum state;
		alGetSourcei(source, AL_SOURCE_STATE, &state);

		if (state != AL_PLAYING) {
			alDeleteSources(1, &source);
			_sources.erase(j++);
			continue;
		}
		if (j->first.first != last_id) {
			last_id = j->first.first;
			o = World->getObjectByID(j->first.first);
		}

		if (o == NULL)  {
			++j;
			continue;
		}
		
		v2<float> pos, vel;
		o->getInfo(pos, vel);
		GET_CONFIG_VALUE("engine.sound.positioning-divisor", float, k, 200.0);
		
		ALfloat al_pos[] = { pos.x / k, -pos.y / k, 0*o->getZ() / k };
		ALfloat al_vel[] = { vel.x / k, -vel.y / k, 0 };
	
		alSourcefv(j->second, AL_POSITION, al_pos);
		alSourcefv(j->second, AL_VELOCITY, al_vel);
		++j;
	} CATCH("updateObjects", {++j; continue;})
	}
}


void IMixer::setListener(const v3<float> &pos, const v3<float> &vel) {
	//LOG_DEBUG(("setListener: %g %g", pos.x, pos.y));
	GET_CONFIG_VALUE("engine.sound.positioning-divisor", float, k, 200.0);
		
	ALfloat al_pos[] = { pos.x / k, -pos.y / k, 0*pos.z / k };
	ALfloat al_vel[] = { vel.x / k, -vel.y / k, 0*vel.z / k };
		
	alListenerfv(AL_POSITION,    al_pos);
	alListenerfv(AL_VELOCITY,    al_vel);
	//alListenerfv(AL_ORIENTATION, al_vel);
}
	



void IMixer::cancelSample(const Object *o, const std::string &name) {
	if (_nosound || name.empty())
		return;
	//LOG_DEBUG(("object %d cancels %s", o->getID(), name.c_str()));
	Sources::iterator j1 = _sources.lower_bound(Sources::key_type(o->getID(), name));
	Sources::iterator j2 = _sources.upper_bound(Sources::key_type(o->getID(), name));
	for(Sources::iterator j = j1; j != j2; ++j) {
		alSourcei (j->second, AL_LOOPING, AL_FALSE);
		AL_CHECK(("alSourcei"));
	}
}

void IMixer::cancelAll(const Object *o) {
	if (_nosound)
		return;
	
	const int id = o->getID();
	for(Sources::iterator j = _sources.begin(); j != _sources.end(); ++j) {
		if (j->first.first == id) {
			alSourcei (j->second, AL_LOOPING, AL_FALSE);
			AL_CHECK(("alSourcei"));
		}
	}
}


void IMixer::cancelAll() {
	if (_nosound)
		return;
	LOG_DEBUG(("stop playing anything"));
	for(Sources::iterator j = _sources.begin(); j != _sources.end(); ++j) {
		alSourceStop(j->second);
		alDeleteSources(1, &j->second);
	}
	_sources.clear();
}
