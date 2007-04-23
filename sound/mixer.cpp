
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

IMPLEMENT_SINGLETON(Mixer, IMixer);

IMixer::IMixer() : _nosound(true), _nomusic(true), _update_objects(true), _ogg(NULL), 
	_volume_fx(1.0f), _volume_music(1.0f) {}

void IMixer::init(const bool nosound, const bool nomusic) {
	if (nosound && nomusic) {
		_nosound = _nomusic = true;
		return;
	}

	Config->get("engine.sound.volume.fx", _volume_fx, 1.0f);
	Config->get("engine.sound.volume.music", _volume_music, 1.0f);
	GET_CONFIG_VALUE("engine.sound.update-objects-interval", float, uoi, 0.1);
	_update_objects.set(uoi);
	
	LOG_DEBUG(("volumes: music: %g, fx: %g", _volume_music, _volume_fx));
	
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

IMixer::~IMixer() {
	_nosound = _nomusic = true;
}


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
	LOG_DEBUG(("loaded %u songs in playlist", (unsigned)_playlist.size()));
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

	_ogg->open(fname, continuous, _volume_music);
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

void IMixer::loadSample(const std::string &filename, const std::string &classname) {
	if (_nosound) 
		return;
	LOG_DEBUG(("loading sample %s", filename.c_str()));
	
	
	if (_sounds.find(filename) != _sounds.end()) {
		//fix classname anyway to allow one sample have multiply classes.
		if (!classname.empty())
			_classes[classname].insert(filename);
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

	if (!classname.empty())
		_classes[classname].insert(filename);
}

void IMixer::playRandomSample(const Object *o, const std::string &classname, const bool loop) {
	if (_nosound || classname.empty())
		return;
	
	Classes::const_iterator i = _classes.find(classname);
	if (i == _classes.end()) {
		LOG_WARN(("no samples class '%s' registered", classname.c_str()));
		return;
	}
	const std::set<std::string> &samples = i->second;
	if (samples.empty()) {
		LOG_WARN(("samples class '%s' has no samples inside. bug.", classname.c_str()));
		return;		
	}
	int n = mrt::random(samples.size());
	std::set<std::string>::const_iterator s = samples.begin();
	while(n-- && s != samples.end())
		++s;

	assert(s != samples.end());
	playSample(o, *s, loop);
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
	const Sample &sample = *(i->second);
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
		alSourcef (source, AL_GAIN,     _volume_fx   );
		alSourcefv(source, AL_POSITION, al_pos       );
		alSourcefv(source, AL_VELOCITY, al_vel       );
		alSourcei (source, AL_LOOPING,  loop?AL_TRUE:AL_FALSE );
		alSourcePlay(source);
	} CATCH("playSample", {});
}

void IMixer::setFXVolume(const float volume) {
	if (volume < 0 || volume > 1) 
		throw_ex(("volume value %g is out of range [0-1]", volume));	

	for(Sources::iterator i = _sources.begin(); i != _sources.end(); ++i) {
		alSourcef(i->second, AL_GAIN, volume);
		AL_CHECK(("alSourcef(AL_GAIN, %g)", volume));
	}

	_volume_fx = volume;
}

void IMixer::setMusicVolume(const float volume) {
	if (volume < 0 || volume > 1) 
		throw_ex(("volume value %g is out of range [0-1]", volume));	
	
	if (_ogg)
		_ogg->setVolume(volume);

	_volume_music = volume;	
}


void IMixer::tick(const float dt) {
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
		
	if (!_update_objects.tick(dt))
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
			alSourcei (source, AL_LOOPING, AL_FALSE);
			++j;
			continue;
		}
		
		v2<float> pos, vel;
		o->getInfo(pos, vel);
		GET_CONFIG_VALUE("engine.sound.positioning-divisor", float, k, 200.0);
		
		ALfloat al_pos[] = { pos.x / k, -pos.y / k, 0*o->getZ() / k };
		ALfloat al_vel[] = { vel.x / k, -vel.y / k, 0 };
	
		alSourcefv(source, AL_POSITION, al_pos);
		alSourcefv(source, AL_VELOCITY, al_vel);
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
