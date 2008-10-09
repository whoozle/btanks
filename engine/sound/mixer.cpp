/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
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

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/


#include "mixer.h"
#include "mrt/file.h"
#include "mrt/exception.h"
#include "mrt/random.h"
#include "mrt/chunk.h"
#include <assert.h>
#include "utils.h"

#include "ogg_stream.h"
#include "world.h"

#include "config.h"
#include "object.h"
#include "math/v3.h"
#include "tmx/map.h"
#include "finder.h"

#include "clunk/sample.h"
#include "clunk/source.h"
#include "clunk/context.h"

IMPLEMENT_SINGLETON(Mixer, IMixer);

void IMixer::reset() {
	if (!_loop)
		return;
	play();
}

IMixer::IMixer() : _nosound(true), _nomusic(true), 
	_volume_fx(1.0f), _volume_ambience(0.5f), _volume_music(1.0f), _debug(false), _loop(false), 
	_context(NULL) {
	}

void IMixer::init(const bool nosound, const bool nomusic) {
	if (nosound && nomusic) {
		_nosound = _nomusic = true;
		return;
	}
	
	Config->get("engine.sound.debug", _debug, false);

	try {	
		_context = new clunk::Context();
	
		int sample_rate, period = 1024;
		Config->get("engine.sound.sample-rate", sample_rate, 22050);
		//Config->get("engine.sound.period", period, 1024);

		_context->init(sample_rate, 2, period);
		
		clunk::DistanceModel dm(clunk::DistanceModel::Exponent, true, 128);
		dm.rolloff_factor = 0.01f;
		Config->get("engine.sound.speed-of-sound", dm.speed_of_sound, 2000.0f);
		Config->get("engine.sound.doppler-factor", dm.doppler_factor, 1.0f);
		
		_context->set_distance_model(dm);
	} CATCH("clunk initialization", { delete _context; _context = NULL; _nomusic = _nosound = true; });
	
	if (_context == NULL)
		return;

	Config->get("engine.sound.volume.fx", _volume_fx, 1.0f);
	Config->get("engine.sound.volume.ambience", _volume_ambience, 0.5f);
	Config->get("engine.sound.volume.music", _volume_music, 1.0f);
	
	LOG_DEBUG(("volumes: music: %g, ambience: %g, fx: %g", _volume_music, _volume_ambience, _volume_fx));
	
	_nosound = nosound;

	_context->set_fx_volume(_volume_fx);
	
	//TRY {
	//	alDistanceModel(AL_EXPONENT_DISTANCE_CLAMPED);
	//	AL_CHECK(("alDistanceModel"));
	//} CATCH("setting distance model", {})
	
	_nomusic = nomusic;
}

void IMixer::deinit() {
	if (_context != NULL) {
		_context->stop_all();
		
		_context->deinit();
		
		std::for_each(_sounds.begin(), _sounds.end(), delete_ptr2<Sounds::value_type>());
		_sounds.clear();

		delete _context;
		_context = NULL;
	}

	_nosound = true;
	_nomusic = true;
}

IMixer::~IMixer() {
	_nosound = _nomusic = true;
}

#include "mrt/scoped_ptr.h"

void IMixer::loadPlaylist(const std::string &file) {
	if (_nomusic) 
		return;
	
	TRY {
		scoped_ptr<mrt::BaseFile> f(Finder->get_file(file, "rt"));
		std::string line;
		while(f->readline(line)) {
			mrt::trim(line);
			_playlist[line] = false;
		}
		f->close();
	} CATCH("loadPlayList", {});
	LOG_DEBUG(("playlist loaded... %u songs in playlist", (unsigned)_playlist.size()));
}

const bool IMixer::play(const std::string &fname, const bool continuous) {
	if (_nomusic || _context == NULL) 
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

	std::string real_file = Finder->find("tunes/" + fname, false);
	if (real_file.empty())
		return false;

	_context->play(0, new OggStream(real_file), continuous);
	_context->set_volume(0, _volume_music);
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
	if (_nosound || _context == NULL) 
		return;
	
	if (_sounds.find(filename) != _sounds.end()) {
		//fix classname anyway to allow one sample have multiply classes.
		if (!classname.empty())
			_classes[classname].insert(filename);
		LOG_DEBUG(("sample %s already loaded, skipped.", filename.c_str()));
		return;
	}

	LOG_DEBUG(("loading sample %s", filename.c_str()));
	

	clunk::Sample * sample = NULL;
	TRY {
		sample = _context->create_sample();
		mrt::Chunk data;
		OggStream::decode(*sample, Finder->find("sounds/" + filename));
		LOG_DEBUG(("sample %s decoded. ", filename.c_str()));
		_sounds[filename] = sample;
	} CATCH("loadSample", { delete sample; sample = NULL; });

	if (!classname.empty())
		_classes[classname].insert(filename);
}

void IMixer::playRandomSample(Object *o, const std::string &classname, const bool loop, const float gain) {
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

void IMixer::playSample(Object *o, const std::string &name, const bool loop, const float gain) {
	if (_nosound || _context == NULL || name.empty())
		return;

TRY {
	//LOG_DEBUG(("object: %d requests %s (%s)", id, name.c_str(), loop?"loop":"single"));
	Sounds::const_iterator i = _sounds.find(name);
	if (i == _sounds.end()) {
		LOG_WARN(("sound %s was not loaded. skipped.", name.c_str()));
		return;
	}
	clunk::Sample *sample = i->second;

	if (o) {
		clunk::Object *clunk_object = o->get_clunk_object();
		if (clunk_object == NULL) {
			clunk_object = _context->create_object();
			o->set_clunk_object(clunk_object);
		}
		
		if (loop && clunk_object->playing(name)) {
			clunk_object->set_loop(name, true);
			return;
		};

		if (_debug)
			LOG_DEBUG(("playSample('%s', %s, %g)", name.c_str(), loop?"loop":"once", _volume_fx * gain));

		
		v2<float> pos = Map->distance(v2<float>(listener_pos.x, listener_pos.y), o->get_position()), vel;
		o->get_velocity(vel);
		
		clunk_object->update(clunk::v3<float>(pos.x, -pos.y, 0), clunk::v3<float>(vel.x, -vel.y, 0), clunk::v3<float>(0, 1, 0));
	
		double pitch = 1;
		GET_CONFIG_VALUE("engine.sound.delta-pitch", float, sdp, 0.019440643702144828169815632631f); //1/3 semitone
		if (!loop) 
			pitch += (double)sdp * (mrt::random(2000) - 1000) / 1000.0;

		if (_debug)
			LOG_DEBUG(("pitch = %g", pitch));
		clunk_object->play(name, new clunk::Source(sample, loop, clunk::v3<float>(), gain, pitch));
	} else {
		if (_debug)
			LOG_DEBUG(("playSample(@listener)('%s', %s, %g)", name.c_str(), loop?"loop":"once", _volume_fx * gain));
		clunk::Object * listener = _context->get_listener();
		if (listener != NULL)
			listener->play(name, new clunk::Source(sample, loop, clunk::v3<float>(), gain));
	}

} CATCH("playSample", { });
}

void IMixer::setFXVolume(const float volume) {
	if (volume < 0 || volume > 1) 
		throw_ex(("volume value %g is out of range [0-1]", volume));	

	if (_context != NULL)
		_context->set_fx_volume(volume);
	_volume_fx = volume;
}

void IMixer::setMusicVolume(const float volume) {
	if (volume < 0 || volume > 1) 
		throw_ex(("volume value %g is out of range [0-1]", volume));	

	if (_context != NULL)
		_context->set_volume(0, volume);
	_volume_music = volume;	
}

void IMixer::setAmbienceVolume(const float volume) {
	if (volume < 0 || volume > 1) 
		throw_ex(("volume value %g is out of range [0-1]", volume));	

	if (_context != NULL)
		_context->set_volume(1, volume);
	_volume_ambience = volume;	
}

void IMixer::tick(const float dt) {
	if (_context != NULL && !_context->playing(0)) {
		//LOG_DEBUG(("sound thread idle"));
		play();
	}
}


void IMixer::set_listener(const v3<float> &pos, const v3<float> &vel, const float r) {
	listener_pos = pos;
	listener_vel = vel;	
}

void IMixer::get_listener(v3<float> &pos, v3<float> &vel, float& r) {
	pos = listener_pos;
	vel = listener_vel;
	r = 0;
}

void IMixer::cancel_all() {
	stopAmbient();
}

void IMixer::startAmbient(const std::string &fname) {
	if (_context == NULL) 
		return;
	TRY {
		_context->play(1, new OggStream(Finder->find("sounds/ambient/" + fname)), true);
		_context->set_volume(1,  _volume_ambience);
	} CATCH("startAmbient", {});
}

void IMixer::stopAmbient() {
	if (_context == NULL) 
		return;
	_context->stop(1);
}
