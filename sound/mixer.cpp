
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

IMixer::SourceInfo::SourceInfo(const int id, const std::string &name, const bool loop) : id(id), name(name), loop(loop) {}

const bool IMixer::SourceInfo::operator<(const SourceInfo &other) const {
	if (id != other.id) 
		return id < other.id;

	if (name != other.name);
		return name < other.name;

	return loop < other.loop;
}


IMixer::IMixer() : _no_more_sources(false), _nosound(true), _nomusic(true), _ogg(NULL), _ambient(NULL), _ogg_source(0),
	_volume_fx(1.0f), _volume_music(1.0f) {}

void IMixer::init(const bool nosound, const bool nomusic) {
	if (nosound && nomusic) {
		_nosound = _nomusic = true;
		return;
	}

	Config->get("engine.sound.volume.fx", _volume_fx, 1.0f);
	Config->get("engine.sound.volume.music", _volume_music, 1.0f);
	
	LOG_DEBUG(("volumes: music: %g, fx: %g", _volume_music, _volume_fx));
	
	delete _ogg;
	_ogg = NULL;
	
	TRY {
		alutInit(NULL, NULL);
		ALUT_CHECK(("alutInit"));

#	ifdef WIN32
		GET_CONFIG_VALUE("engine.sound.preallocate-sources", bool, preallocate, true);
#	else
		GET_CONFIG_VALUE("engine.sound.preallocate-sources", bool, preallocate, false);
#	endif
		if (preallocate) {
			LOG_DEBUG(("preallocate sources..."));
			GET_CONFIG_VALUE("engine.sound.maximum-sources", int, max_sources, 32);
			
			_no_more_sources = true;
			ALuint *sources = new ALuint[max_sources];
			int n;
			
			for(n = max_sources; n >= 4; --n) {
				alGenSources(n, sources);
				if (alGetError() == AL_NO_ERROR)
					break;
			}
			if (n < 4) {
				delete[] sources;
				throw_ex(("cannot generate enough sources."));
			}
		
			LOG_DEBUG(("%d sources allocated.", n));
			for(int i = 0; i < n; ++i) {
				_free_sources.insert(sources[i]);
			}
			delete[] sources;
		}
		if (!nomusic) {
			if (!generateSource(_ogg_source))
				throw_ex(("cannot generate source for music stream"));
		}
		if (!generateSource(_ambient_source))
			throw_ex(("cannot generate source for ambient stream"));
		
	} CATCH("alutInit", {
		LOG_DEBUG(("there was error(s) during initialization, disabling sounds."));
		_nosound = _nomusic = true;
		return;
	})	
	
	TRY {
		GET_CONFIG_VALUE("engine.sound.doppler-factor", float, df, 1.0);
		alDopplerFactor(df);
		AL_CHECK(("alDopplerFactor"));
	
		GET_CONFIG_VALUE("engine.sound.speed-of-sound", float, sos, 2000);
		alSpeedOfSound(sos);
		AL_CHECK(("setting speed of sound"));
	} CATCH("init", {});

	_nosound = nosound;
	
	TRY {
		if (!nomusic)
			_ogg = new OggStream(_ogg_source);
	} CATCH("music thread startup", { _nomusic = true; return; })

	TRY {
		if (!nosound)
			_ambient = new OggStream(_ambient_source);
	} CATCH("ambient thread startup", {})
	
	//TRY {
	//	alDistanceModel(AL_EXPONENT_DISTANCE_CLAMPED);
	//	AL_CHECK(("alDistanceModel"));
	//} CATCH("setting distance model", {})
	
	_nomusic = nomusic;
}

void IMixer::deinit() {
	LOG_DEBUG(("cleaning up mixer..."));	
	delete _ogg; 
	_ogg = NULL;
	delete _ambient; 
	_ambient = NULL;
	
	for(std::set<ALuint>::iterator i = _free_sources.begin(); i != _free_sources.end(); ++i) {
		alDeleteSources(1, &*i);
	}
	
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
	if (_ogg == NULL) 
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

	_ogg->play(fname, continuous, _volume_music);
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

void IMixer::playRandomSample(const Object *o, const std::string &classname, const bool loop, const float gain) {
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
	playSample(o, *s, loop, gain);
}

const bool IMixer::generateSource(ALuint &source) {
	//always return generated source first
	if (!_free_sources.empty()) {
		source = *_free_sources.begin();
		_free_sources.erase(_free_sources.begin());
		LOG_DEBUG(("source %08x has been taken from free sources.", (unsigned)source));
		return true;
	}
	
	if (!_no_more_sources) { //if we was not reached upper sources limit, generate one more source.
		alGenSources(1, &source);
		LOG_DEBUG(("source %08x was generated by alGetSources(1).", (unsigned)source));
		if (alGetError() == AL_NO_ERROR)
			return true;
	
		_no_more_sources = true; //oops.
		LOG_DEBUG(("sources limit reached. dynamic sources: %u", (unsigned)(_free_sources.size() + _sources.size())));
	}
	
	//LOG_DEBUG(("searching source victim."));
	ALfloat l_pos[] = { 0, 0, 0 };
	alGetListenerfv(AL_POSITION, l_pos);
	AL_CHECK(("alGetListenerfv(AL_POSITION)"));
	LOG_DEBUG(("finding distant sources, listener position : %g %g %g", (float)l_pos[0], (float)l_pos[1], (float)l_pos[2]));
	
	v2<float> listener_pos((float)l_pos[0], (float)l_pos[1]);

	Sources::iterator victim = _sources.end();
	float max_d = 0;
	for(Sources::iterator i = _sources.begin(); i != _sources.end(); ++i) {
	TRY {
		const SourceInfo &info = i->first;
		if (info.loop)
			continue;

		ALenum state = 0;
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		ALenum r = alGetError();

		if (r != AL_NO_ERROR || state != AL_PLAYING) {
			if (r != AL_NO_ERROR)
				LOG_ERROR(("alGetSourcei(%08x, AL_SOURCE_STATE): error %08x, state returned: %08xx", source, (unsigned)r, (unsigned)state));
			victim = i;
			break;
		}

		ALfloat s_pos[] = { 0, 0, 0 };
		alGetSourcefv(i->second, AL_POSITION, s_pos);
		AL_CHECK(("alGetSourcefv(%08x, AL_POSITION)", (unsigned)i->second));
		v2<float> source_pos((float)s_pos[0], (float)s_pos[1]);
		float d = source_pos.distance(listener_pos);
		LOG_DEBUG(("source position : %g %g %g, distance = %g", (float)s_pos[0], (float)s_pos[1], (float)s_pos[2], d));
		if (d > max_d) {
			max_d = d;
			victim = i;
		}
	} CATCH(mrt::formatString("id %d, sound: %s", i->first.id, i->first.name.c_str()).c_str(), );
	}
	if (victim != _sources.end()) {
		source = victim->second;
		LOG_DEBUG(("killing source %08x with distance %g", (unsigned)source, max_d));
		alSourceStop(source);
		AL_CHECK_NON_FATAL(("alSourceStop(%08x)", source));
		_sources.erase(victim);
		return true;
	}
	
	LOG_WARN(("cannot allocate any sources."));
	return false;
}

void IMixer::deleteSource(const ALuint source) {
	//alDeleteSources(1, &source);
	alSourceStop(source);
	AL_CHECK_NON_FATAL(("alSourceStop(%08x)", source));
	alSourcei(source, AL_BUFFER, AL_NONE);
	AL_CHECK_NON_FATAL(("alSourcei(%08x, AL_BUFFER, AL_NONE)", source));
	
	_free_sources.insert(source);
	LOG_DEBUG(("source %08x freed", (unsigned)source));
}

void IMixer::playSample(const Object *o, const std::string &name, const bool loop, const float gain) {
	if (_nosound || name.empty())
		return;

	const int id = (o)?o->getID():0;
	//LOG_DEBUG(("object: %d requests %s (%s)", id, name.c_str(), loop?"loop":"single"));
	Sounds::const_iterator i = _sounds.find(name);
	if (i == _sounds.end()) {
		LOG_WARN(("sound %s was not loaded. skipped.", name.c_str()));
		return;
	}

	GET_CONFIG_VALUE("engine.sound.positioning-divisor", float, k, 40.0);

	v2<float> pos, vel;
	if (o) {
		o->getInfo(pos, vel);
		
		ALfloat l_pos[] = { 0, 0, 0 };
		alGetListenerfv(AL_POSITION, l_pos);
		AL_CHECK(("alGetListenerfv(AL_POSITION)"));

		v2<float> listener_pos((float)l_pos[0], (float)l_pos[1]), source_pos = pos;
		
		//LOG_DEBUG(("listener position : %g %g %g", (float)l_pos[0], (float)l_pos[1], (float)l_pos[2]));
		source_pos.x /= k;
		source_pos.y /= -k;
		
		GET_CONFIG_VALUE("engine.sound.maximum-distance", float, md, 60.0f);
		float d = source_pos.distance(listener_pos);
		if (!loop && d > md) {
			LOG_DEBUG(("sound %s was skipped (distance: %g)", name.c_str(), d));
			return;
		}
	}
	
	const Sample &sample = *(i->second);
	ALuint source;
	if (!generateSource(source)) {
		LOG_WARN(("cannot generate source. skip sound %s", name.c_str()));
		return;
	}
	TRY {
		LOG_DEBUG(("playSample('%s', %s, %g)", name.c_str(), loop?"loop":"once", _volume_fx * gain));
		
		if (o) {
			ALfloat al_pos[] = { pos.x / k, -pos.y / k, 0*o->getZ() / k };
			ALfloat al_vel[] = { vel.x / k, -vel.y / k, 0 };
			alSourcefv(source, AL_POSITION, al_pos       );
			AL_CHECK(("alSourcefv(%08x, AL_POSITION)", source));
			alSourcefv(source, AL_VELOCITY, al_vel       );
			AL_CHECK(("alSourcefv(%08x, AL_VELOCITY)", source));
			alSourcef (source, AL_ROLLOFF_FACTOR,  1.0   );
			AL_CHECK(("alSourcef(%08x, AL_ROLLOFF_FACTOR, 1.0)", source));
			alSourcei (source, AL_SOURCE_RELATIVE, AL_FALSE     );
			AL_CHECK(("alSourcei(%08x, AL_SOURCE_RELATIVE, AL_FALSE)", source));
		} else {
			alSource3f(source, AL_POSITION,        0.0, 0.0, 0.0);
			AL_CHECK(("alSource3f(%08x, AL_POSITION)", source));
			alSource3f(source, AL_VELOCITY,        0.0, 0.0, 0.0);
			AL_CHECK(("alSource3f(%08x, AL_VELOCITY)", source));
			alSource3f(source, AL_DIRECTION,       0.0, 0.0, 0.0);
			AL_CHECK(("alSource3f(%08x, AL_DIRECTION)", source));
				
			alSourcef (source, AL_ROLLOFF_FACTOR,  0.0          );
			AL_CHECK(("alSourcef(%08x, AL_ROLLOFF_FACTOR, 0.0)", source));
			alSourcei (source, AL_SOURCE_RELATIVE, AL_TRUE      );
			AL_CHECK(("alSourcei(%08x, AL_SOURCE_RELATIVE, AL_TRUE)", source));
		}
	
		//alSourcef (source, AL_REFERENCE_DISTANCE, (o->size.x + o->size.y) / k / 2);
		//GET_CONFIG_VALUE("engine.sound.maximum-distance", float, max_dist, 800.0);
		//float max_dist_al = max_dist / k;
				
		alSourcei (source, AL_BUFFER,   sample.buffer);
		AL_CHECK(("alSourcei(%08x, AL_BUFFER, %08x)", (unsigned)source, (unsigned)sample.buffer));
		
		alSourcef (source, AL_PITCH,    1.0          );
		AL_CHECK(("alSourcef(%08x, AL_PITCH, 1.0)", source));
		alSourcef (source, AL_GAIN,     _volume_fx * gain  );
		AL_CHECK(("alSourcef(%08x, AL_GAIN, %g)", source, _volume_fx * gain));
		alSourcei (source, AL_LOOPING,  loop?AL_TRUE:AL_FALSE );
		AL_CHECK(("alSourcei(%08x, AL_LOOPING, %s)", source, loop?"AL_TRUE":"AL_FALSE"));

		_sources.insert(Sources::value_type(Sources::key_type(id, name, loop), source));

		TRY {
			alSourcePlay(source);
			AL_CHECK(("alSourcePlay(%08x, '%s', %s)", (unsigned)source, name.c_str(), loop?"loop":"once"));
		} CATCH("playSound", {});
	
	} CATCH("playSample", {deleteSource(source);});
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


void IMixer::updateObject(const Object *o) {
	v2<float> pos, vel;
	o->getInfo(pos, vel);
	GET_CONFIG_VALUE("engine.sound.positioning-divisor", float, k, 40.0);
	
	ALfloat al_pos[] = { pos.x / k, -pos.y / k, 0*o->getZ() / k };
	ALfloat al_vel[] = { vel.x / k, -vel.y / k, 0 };
	
	const int id = o->getID();

	for(Sources::iterator i = _sources.begin(); i != _sources.end(); ++i) {
		if (i->first.id == id) {
			ALuint source = i->second;
			alSourcefv(source, AL_POSITION, al_pos);
			AL_CHECK_NON_FATAL(("alSourcefv(%08x, AL_POSITION, {%g,%g,%g})", source, al_pos[0], al_pos[1], al_pos[2] ));
			alSourcefv(source, AL_VELOCITY, al_vel);
			AL_CHECK_NON_FATAL(("alSourcefv(%08x, AL_VELOCITY, {%g,%g,%g})", source, al_vel[0], al_vel[1], al_vel[2] ));
		}
	}
}

void IMixer::tick(const float dt) {
	if (_ogg != NULL && _ogg->idle()) {
		//LOG_DEBUG(("sound thread idle"));
		play();
	}

	if (_nosound) 
		return;
		
	for(Sources::iterator j = _sources.begin(); j != _sources.end();) {
	TRY {
		ALuint source = j->second;
		ALenum state;
		
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		ALenum r = alGetError();

		if (r != AL_NO_ERROR || state != AL_PLAYING) {
			if (r != AL_NO_ERROR)
				LOG_ERROR(("alGetSourcei(%08x, AL_SOURCE_STATE): error %08x", source, (unsigned)r));
			deleteSource(source);
			_sources.erase(j++);
			continue;
		}
		++j;
	} CATCH("updateObjects", {++j; continue;})
	}
}


void IMixer::setListener(const v3<float> &pos, const v3<float> &vel, const float r) {
	//LOG_DEBUG(("setListener: %g %g", pos.x, pos.y));
	GET_CONFIG_VALUE("engine.sound.positioning-divisor", float, k, 40.0);
		
	ALfloat al_pos[] = { pos.x / k, -pos.y / k, 0*pos.z / k };
	ALfloat al_vel[] = { vel.x / k, -vel.y / k, 0*vel.z / k };
		
	alListenerfv(AL_POSITION,    al_pos);
	AL_CHECK_NON_FATAL(("alListenerfv(AL_POSITION, {%g,%g,%g})", al_pos[0], al_pos[1], al_pos[2] ));
	alListenerfv(AL_VELOCITY,    al_vel);
	AL_CHECK_NON_FATAL(("alListenerfv(AL_VELOCITY, {%g,%g,%g})", al_vel[0], al_vel[1], al_vel[2] ));
	//alListenerf (AL_REFERENCE_DISTANCE, r / k);
	//alListenerfv(AL_ORIENTATION, al_vel);
}
	



void IMixer::cancelSample(const Object *o, const std::string &name) {
	if (_nosound || name.empty())
		return;
	//LOG_DEBUG(("object %d cancels %s", o->getID(), name.c_str()));
	for(Sources::iterator j = _sources.begin(); j != _sources.end(); ++j) {
		const SourceInfo &info = j->first;
		if (info.id != o->getID() || info.name != name || !info.loop)
			continue;
		
		alSourcei (j->second, AL_LOOPING, AL_FALSE);
		AL_CHECK(("alSourcei"));
	}
}

void IMixer::cancelAll(const Object *o) {
	if (_nosound)
		return;
	
	const int id = o->getID();
	for(Sources::iterator j = _sources.begin(); j != _sources.end(); ++j) {
		if (j->first.id == id && j->first.loop) {
			alSourcei (j->second, AL_LOOPING, AL_FALSE);
			AL_CHECK(("alSourcei"));
		}
	}
}


void IMixer::cancelAll() {
	stopAmbient();
	
	if (_nosound)
		return;

	if (!_sources.empty()) {
		LOG_DEBUG(("stop playing anything"));
		for(Sources::iterator j = _sources.begin(); j != _sources.end(); ++j) {
			deleteSource(j->second);
		}
	}
	_sources.clear();
}

void IMixer::startAmbient(const std::string &fname) {
	if (_nosound) 
		return;
	TRY {
		if (_ambient)
			_ambient->play(Finder->find("sounds/ambient/" + fname), true, 1.0f);
	} CATCH("startAmbient", {});
}

void IMixer::stopAmbient() {
	if (_ambient == NULL) 
		return;
	_ambient->stop();
}
