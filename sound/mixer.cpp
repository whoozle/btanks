
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

#include "config.h"
#include "object.h"
#include "math/v3.h"
#include "finder.h"

IMPLEMENT_SINGLETON(Mixer, IMixer);

IMixer::SourceInfo::SourceInfo(const std::string &name, const bool loop, const ALuint source) : 
	name(name), loop(loop), source(source), persistent(false) {}
	
void IMixer::SourceInfo::updatePV() {
	if (source == AL_NONE)
		return;

	ALfloat al_pos[] = { pos.x, pos.y, pos.z };
	ALfloat al_vel[] = { vel.x, vel.y, vel.z };

	alSourcefv(source, AL_POSITION, al_pos);
	AL_CHECK_NON_FATAL(("alSourcefv(%08x, AL_POSITION, {%g,%g,%g})", source, al_pos[0], al_pos[1], al_pos[2] ));
	alSourcefv(source, AL_VELOCITY, al_vel);
	AL_CHECK_NON_FATAL(("alSourcefv(%08x, AL_VELOCITY, {%g,%g,%g})", source, al_vel[0], al_vel[1], al_vel[2] ));	
}

const bool IMixer::SourceInfo::playing() const {
	assert(source != AL_NONE);

	ALenum state = 0;

	alGetSourcei(source, AL_SOURCE_STATE, &state);
	ALenum r = alGetError();

	if (r != AL_NO_ERROR || state != AL_PLAYING) {
		if (r != AL_NO_ERROR)
				LOG_ERROR(("alGetSourcei(%08x, AL_SOURCE_STATE): error %08x", source, (unsigned)r));
		return false;
	}
/*	
	ALint n = 0;
	alGetSourcei(source, AL_BUFFERS_PROCESSED, &n);
	r = alGetError();
	LOG_DEBUG(("alGetSourcei(%08x, AL_BUFFERS_PROCESSED, %d): %s", (unsigned)source, n, r == AL_NO_ERROR?"ok":"error"));
*/	
	return true;
}

void IMixer::reset() {
	if (!_loop)
		return;
	play();
}

IMixer::IMixer() : alc_device(NULL), alc_context(NULL), 
	_no_more_sources(false), _nosound(true), _nomusic(true), _ogg(NULL), _ambient(NULL), _ogg_source(0),
	_volume_fx(1.0f), _volume_music(1.0f), _debug(false), _loop(false) {}

void IMixer::dumpContextAttrs(std::map<const std::string, int> & attrs) const {
	ALCint attrSize;
	ALCint *attributes;
	ALCint *data;

	alcGetIntegerv(alc_device, ALC_ATTRIBUTES_SIZE, sizeof(attrSize), &attrSize);
	attributes = (ALCint *)malloc(attrSize * sizeof(ALCint));
	alcGetIntegerv(alc_device, ALC_ALL_ATTRIBUTES, attrSize, attributes);
	LOG_DEBUG(("context attrs length : %d", attrSize));
	data = attributes;
	while (data < attributes + attrSize) {
		switch (*data) {
			case ALC_FREQUENCY:
				++data;
				attrs["ALC_FREQUENCY"] = *data;
				LOG_DEBUG(("ALC_FREQUENCY = %d", *data));
				break;
			case ALC_REFRESH:
				++data;
				attrs["ALC_REFRESH"] = *data;
				LOG_DEBUG(("ALC_REFRESH = %d", *data));
				break;
			case ALC_SYNC:
				++data;
				attrs["ALC_SYNC"] = *data;
				LOG_DEBUG(("ALC_SYNC = %d", *data));
				break;
			case ALC_MONO_SOURCES:
				++data;
				attrs["ALC_MONO_SOURCES"] = *data;
				LOG_DEBUG(("ALC_MONO_SOURCES = %d", *data));
				break;
			case ALC_STEREO_SOURCES:
				++data;
				attrs["ALC_STEREO_SOURCES"] = *data;
				LOG_DEBUG(("ALC_STEREO_SOURCES = %d", *data));
				break;
			default:
				break;
		}
		++data;
	}
	free(attributes);
}

void IMixer::init(const bool nosound, const bool nomusic) {
	if (nosound && nomusic) {
		_nosound = _nomusic = true;
		return;
	}
	
	Config->get("engine.sound.debug", _debug, false);

	Config->get("engine.sound.volume.fx", _volume_fx, 1.0f);
	Config->get("engine.sound.volume.music", _volume_music, 1.0f);
	
	LOG_DEBUG(("volumes: music: %g, fx: %g", _volume_music, _volume_fx));
	
	delete _ogg;
	_ogg = NULL;
	
	TRY {
		ALCint major = 0, minor = 0;
		alcGetIntegerv(NULL, ALC_MAJOR_VERSION, sizeof(major), &major);
		alcGetIntegerv(NULL, ALC_MINOR_VERSION, sizeof(minor), &minor);
		LOG_NOTICE(("openAL version: %d.%d", major, minor));

		GET_CONFIG_VALUE("engine.sound.device", std::string, device, std::string());
		
		std::vector<std::string> device_list;
		const ALchar *name = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
		while(name[0]) {
			std::string sname = name;
			bool blacklisted = false;

			//nvidia's nvopenal is a real crap. 
			if (sname.find("NVIDIA") != sname.npos) {
				blacklisted = true;
				if (device.empty())
					device = "Generic Software";
			}
			//place for other openal implementations ;))))
			LOG_NOTICE(("found device: \"%s\"%s", name, blacklisted?" (blacklisted)":""));
			if (!blacklisted)
				device_list.push_back(sname);
			
			name += sname.size() + 1;
		}
		
		//LOG_NOTICE(("default device reported by openal: \"%s\"", alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER)));
		

		if (device.empty()) {
			//autoconfiguring
			if (!device_list.empty()) {
				device = device_list[0];
			}
		}
		if (device.empty()) {
			LOG_DEBUG(("using default device"));
		} else {
			LOG_DEBUG(("using device \"%s\"", device.c_str()));		
		}
		alc_device = device.empty()? alcOpenDevice(NULL): alcOpenDevice(device.c_str());
		if (alc_device == NULL)
			throw_ex(("alcOpenDevice failed: no device '%s' found ('' means default(NULL) device)", device.c_str()));

		LOG_NOTICE(("opened device: %s", alcGetString(alc_device, ALC_DEVICE_SPECIFIER)));
		LOG_NOTICE(("extensions: %s", alcGetString(alc_device, ALC_EXTENSIONS)));

		GET_CONFIG_VALUE("engine.sound.openal-sync-context", bool, sync_ctx, false);
		GET_CONFIG_VALUE("engine.sound.openal-refresh-frequency", int, refresh, 15); //openal default
		GET_CONFIG_VALUE("engine.sound.openal-stereo-sources-hint", int, stereo, 2);
		GET_CONFIG_VALUE("engine.sound.maximum-sources", int, max_sources, 16);
		
		ALCint attrs[] = {
			ALC_SYNC, sync_ctx?AL_TRUE:AL_FALSE, 
			ALC_REFRESH, refresh,
			ALC_STEREO_SOURCES, stereo,
			ALC_MONO_SOURCES, max_sources,
			ALC_INVALID, ALC_INVALID, 
		};
		
		alc_context = alcCreateContext(alc_device, attrs);
		if (alc_context == NULL) 
			throw_ex(("alcCreateContext failed"));
		

		if (alcMakeContextCurrent(alc_context) == ALC_FALSE) 
			throw_ex(("alcMakeContextCurrent(%p) failed", (void *)alc_context));
			
		std::map<const std::string, int> context_attrs;
		dumpContextAttrs(context_attrs);
		int context_sources = 0;
		if (context_attrs["ALC_MONO_SOURCES"]) {
			 context_sources = context_attrs["ALC_MONO_SOURCES"] + context_attrs["ALC_STEREO_SOURCES"];
		}

		if (context_sources) {
			LOG_DEBUG(("device reported %d sources", context_sources));
			max_sources = context_sources;
		} else {
			LOG_DEBUG(("no ALC_MONO_SOURCES, fallback to generic values..."));
		}

#	ifdef WIN32
		GET_CONFIG_VALUE("engine.sound.preallocate-sources", bool, preallocate, true);
#	else
		GET_CONFIG_VALUE("engine.sound.preallocate-sources", bool, preallocate, false);
#	endif
		if (preallocate) {
			LOG_DEBUG(("preallocating sources..."));
			
			_no_more_sources = true;
			ALuint *sources = new ALuint[max_sources];
			int n;
			
			for(n = max_sources; n >= 1; --n) {
				alGenSources(n, sources);
				if (alGetError() == AL_NO_ERROR)
					break;
			}
			if (n < 2) {
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
	
	if (alc_context != NULL) {
		LOG_DEBUG(("destroying openAL context..."));
		if (alcMakeContextCurrent(NULL) == ALC_FALSE) 
			LOG_WARN(("alcMakeContextCurrent(%p) failed", (void *)alc_context));
		
		alcDestroyContext(alc_context);
		alc_context = NULL;
	}

	if (alc_device != NULL) {		
		LOG_DEBUG(("destroying openAL device..."));
		if (alcCloseDevice(alc_device) == ALC_FALSE) {
			LOG_WARN(("alcCloseDevice(%p)", (void *) alc_device));
		}
		alc_device = NULL;
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
	LOG_DEBUG(("playlist loaded... %u songs in playlist", (unsigned)_playlist.size()));
}

const bool IMixer::play(const std::string &fname, const bool continuous) {
	if (_ogg == NULL) 
		return false;

	_loop = continuous;	
	
	LOG_DEBUG(("playing %s",fname.c_str()));
	std::string::size_type dp = fname.rfind('.');
	std::string ext = "unknown";
	if (dp != std::string::npos)
		ext = fname.substr(dp + 1);
	
	if (ext != "ogg") {
		LOG_WARN(("cannot play non-ogg files(%s). fixme.", ext.c_str()));
		return false;
	}

	std::string real_file;
	if (!mrt::FSNode::exists(fname)) {
		TRY { 
			real_file = Finder->find("tunes/" + fname);
		} CATCH("finding tune", return false;)
	} else real_file = fname;

	_ogg->play(real_file, continuous, _volume_music);
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

const bool IMixer::generateSource(ALuint &r_source) {
	//always return generated source first
	if (!_free_sources.empty()) {
		r_source = *_free_sources.begin();
		_free_sources.erase(_free_sources.begin());
		if (_debug)
			LOG_DEBUG(("source %08x was taken from free sources.", (unsigned)r_source));
		return true;
	}
	
	if (!_no_more_sources) { //if we was not reached upper sources limit, generate one more source.
		alGenSources(1, &r_source);
		if (_debug)
			LOG_DEBUG(("source %08x was generated by alGetSources(1).", (unsigned)r_source));
		if (alGetError() == AL_NO_ERROR)
			return true;
	
		_no_more_sources = true; //oops.
		LOG_DEBUG(("sources limit reached. dynamic sources: %u", (unsigned)(_free_sources.size() + _sources.size())));
	}
	//if (_debug)
	//	LOG_DEBUG(("searching source victim."));
	ALfloat l_pos[] = { 0, 0, 0 };
	alGetListenerfv(AL_POSITION, l_pos);
	AL_CHECK(("alGetListenerfv(AL_POSITION)"));
	
	if (_debug)
		LOG_DEBUG(("finding distant sources, listener position : %g %g %g", (float)l_pos[0], (float)l_pos[1], (float)l_pos[2]));
	
	v2<float> listener_pos((float)l_pos[0], (float)l_pos[1]);

	Sources::iterator victim = _sources.end();
	float max_d = 0;
	for(Sources::iterator i = _sources.begin(); i != _sources.end(); ++i) {
	const SourceInfo &info = i->second;
	TRY {
		if (info.source == AL_NONE || info.persistent == true)
			continue;

		if (!info.playing()) {
			victim = i;
			break;
		}

		ALfloat s_pos[] = { 0, 0, 0 };
		alGetSourcefv(info.source, AL_POSITION, s_pos);
		AL_CHECK(("alGetSourcefv(%08x, AL_POSITION)", (unsigned)info.source));
		v2<float> source_pos((float)s_pos[0], (float)s_pos[1]);
		float d = source_pos.distance(listener_pos);

		if (_debug)
			LOG_DEBUG(("source position : %g %g %g, distance = %g", (float)s_pos[0], (float)s_pos[1], (float)s_pos[2], d));
		
		if (d > max_d) {
			max_d = d;
			victim = i;
		}
	} CATCH(mrt::formatString("source %08x, sound: %s", (unsigned)info.source, info.name.c_str()).c_str(), );
	}
	if (victim != _sources.end()) {
		SourceInfo &victim_info = victim->second;
		r_source = victim_info.source;
		assert(r_source != AL_NONE);
		if (_debug)
			LOG_DEBUG(("killing source %08x ('%s') with distance %g", (unsigned)r_source, victim_info.name.c_str(), max_d));
		alSourceStop(r_source);
		AL_CHECK_NON_FATAL(("alSourceStop(%08x)", r_source));

		if (victim_info.loop) {
			victim_info.source = AL_NONE;
		} else {
			_sources.erase(victim);
		}
		return true;
	}
	
	LOG_WARN(("cannot allocate any sources."));
	return false;
}

void IMixer::deleteSource(const ALuint source) {
	if (source == AL_NONE)
		return;
	
	//alDeleteSources(1, &source);
	alSourceStop(source);
	AL_CHECK_NON_FATAL(("alSourceStop(%08x)", source));
	alSourcei(source, AL_BUFFER, AL_NONE);
	AL_CHECK_NON_FATAL(("alSourcei(%08x, AL_BUFFER, AL_NONE)", source));
	
	_free_sources.insert(source);
	if (_debug)
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
		if (d > md) {
			if (_debug)
				LOG_DEBUG(("sound %s was skipped (distance: %g)", name.c_str(), d));
			if (loop) 
				_sources.insert(Sources::value_type(id, SourceInfo(name, loop, AL_NONE)));
			return;
		}
	}
	
	purgeInactiveSources();
	
	const Sample &sample = *(i->second);
	ALuint source;
	if (!generateSource(source)) {
		LOG_WARN(("cannot generate source. skip sound %s", name.c_str()));
		return;
	}
	TRY {
		if (_debug)
			LOG_DEBUG(("playSample('%s', %s, %g)", name.c_str(), loop?"loop":"once", _volume_fx * gain));

		alSourcei (source, AL_BUFFER,   sample.buffer);
		AL_CHECK(("alSourcei(%08x, AL_BUFFER, %08x)", (unsigned)source, (unsigned)sample.buffer));
		double pitch = 1.0;
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
			GET_CONFIG_VALUE("engine.sound.delta-pitch", float, sdp, 0.019440643702144828169815632631f); //1/3 semitone
			pitch = 1.0 + (double)sdp * (mrt::random(2000) - 1000) / 1000.0;
			if (_debug)
				LOG_DEBUG(("pitch = %g", pitch));
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
			
		alSourcef (source, AL_PITCH, pitch);
		AL_CHECK(("alSourcef(%08x, AL_PITCH, 1.0)", source));
		alSourcef (source, AL_GAIN,     _volume_fx * gain  );
		AL_CHECK(("alSourcef(%08x, AL_GAIN, %g)", source, _volume_fx * gain));
		alSourcei (source, AL_LOOPING,  loop?AL_TRUE:AL_FALSE );
		AL_CHECK(("alSourcei(%08x, AL_LOOPING, %s)", source, loop?"AL_TRUE":"AL_FALSE"));

		Sources::iterator result = _sources.insert(Sources::value_type(id, SourceInfo(name, loop, source)));
		result->second.persistent = o == NULL;

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
		const SourceInfo &info = i->second;
		if (info.source == AL_NONE)
			continue;
		alSourcef(info.source, AL_GAIN, volume);
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
	
	const v3<float> al_pos( pos.x / k, -pos.y / k, 0*o->getZ() / k ), al_vel( vel.x / k, -vel.y / k, 0);
	
	const int id = o->getID();

	Sources::iterator b = _sources.lower_bound(id);
	Sources::iterator e = _sources.upper_bound(id);
	for(Sources::iterator i = b; i != e; ++i) {
		SourceInfo &info = i->second;
		info.pos = al_pos;
		info.vel = al_vel;
		info.updatePV();
	}
}

const unsigned IMixer::purgeInactiveSources() {
	unsigned none_src = 0;
	for(Sources::iterator j = _sources.begin(); j != _sources.end();) {
	TRY {
		const SourceInfo &info = j->second;
		ALuint source = info.source;
		if (source == AL_NONE) {
			//throw away non loop AL_NONE sources
			if (!info.loop) {
				_sources.erase(j++);
				continue;
			} else ++none_src; // else LOG_DEBUG(("cancelled loop %d: %s", j->first, info.name.c_str()));
			++j;
			continue;
		}
		
		//if (_debug)
		//	LOG_DEBUG(("purgeInactiveSources: %d:%s:%08x", j->first, info.name.c_str(), (unsigned)state));

		if (!info.playing()) {
			deleteSource(source);
			_sources.erase(j++);
			continue;
		}
		++j;
	} CATCH("updateObjects", {++j; continue;})
	}
	return none_src;
}

void IMixer::tick(const float dt) {
	if (_ogg != NULL && _ogg->idle()) {
		//LOG_DEBUG(("sound thread idle"));
		play();
	}

	if (!_nosound)  {
	
	const unsigned none_src = purgeInactiveSources();
	
	if (none_src != 0 && !_free_sources.empty()) {
		if (_debug)
			LOG_DEBUG(("recovering lost loops..."));
		
		ALfloat l_pos[] = { 0, 0, 0 };
		alGetListenerfv(AL_POSITION, l_pos);
		AL_CHECK(("alGetListenerfv(AL_POSITION)"));
		v3<float> listener(l_pos[0], l_pos[1], l_pos[2]);
		GET_CONFIG_VALUE("engine.sound.maximum-distance", float, md, 60.0f);

		for(Sources::iterator j = _sources.begin(); j != _sources.end() && !_free_sources.empty(); ++j) {
		SourceInfo &info = j->second;
		TRY {
			if (info.source != AL_NONE || !info.loop) 
				continue;
			
			if (info.pos.quick_distance(listener) < md) {
				Sounds::const_iterator si = _sounds.find(info.name);
				assert(si != _sounds.end());
				const Sample &sample = *si->second;
				
				info.source = *_free_sources.begin();
				
				alSourcei (info.source, AL_BUFFER,   sample.buffer);
				AL_CHECK(("alSourcei(%08x, AL_BUFFER, %08x)", (unsigned)info.source, (unsigned)sample.buffer));
		
				alSourcei (info.source, AL_LOOPING, AL_TRUE);
				AL_CHECK(("alSourcei(%08x, AL_LOOPING, AL_TRUE)", (unsigned)info.source));

				alSourcePlay(info.source);
				AL_CHECK(("alSourcePlay(%08x, '%s', resume)", (unsigned)info.source, info.name.c_str()));
						
				_free_sources.erase(_free_sources.begin());
			}
		} CATCH("recovering loops", {info.source = AL_NONE; throw;} );
		}
	}

	}
	
	if (!_nomusic || !_nosound) {
		alcProcessContext(alc_context);
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
	
	if (_debug)
		LOG_DEBUG(("object %d cancels %s", o->getID(), name.c_str()));

	const int id = o->getID();
	Sources::iterator b = _sources.lower_bound(id);
	Sources::iterator e = _sources.upper_bound(id);
	for(Sources::iterator i = b; i != e; ++i) {
		SourceInfo &info = i->second;
		if (info.name != name || info.source == AL_NONE)
			continue;
		
		info.loop = false;
		alSourcei (info.source, AL_LOOPING, AL_FALSE);
		AL_CHECK(("alSourcei"));
	}
}

void IMixer::cancelAll(const Object *o) {
	if (_nosound)
		return;
	
	const int id = o->getID();
	Sources::iterator b = _sources.lower_bound(id);
	Sources::iterator e = _sources.upper_bound(id);
	for(Sources::iterator i = b; i != e; ++i) {
		SourceInfo &info = i->second;
		if (info.source == AL_NONE)
			continue;
		
		info.loop = false;
		alSourcei (info.source, AL_LOOPING, AL_FALSE);
		AL_CHECK(("alSourcei"));
	}
}


void IMixer::cancelAll() {
	stopAmbient();
	
	if (_nosound)
		return;

	if (!_sources.empty()) {
		LOG_DEBUG(("stop playing anything"));
		for(Sources::iterator j = _sources.begin(); j != _sources.end(); ++j) {
			deleteSource(j->second.source);
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
