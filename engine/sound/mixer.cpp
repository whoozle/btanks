
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


#include "mixer.h"
#include "mrt/file.h"
#include "mrt/exception.h"
#include "mrt/random.h"
#include "mrt/chunk.h"
#include <assert.h>
#include "utils.h"

#include "ogg_stream.h"
#include "sample.h"

#include "config.h"
#include "object.h"
#include "math/v3.h"
#include "finder.h"
#include "sdlx/sdl_ex.h"

IMPLEMENT_SINGLETON(Mixer, IMixer);

void IMixer::reset() {
	if (!_loop)
		return;
	play();
}

IMixer::IMixer() : _nosound(true), _nomusic(true), _ogg(NULL), _ambient(NULL), 
	_volume_fx(1.0f), _volume_ambience(0.5f), _volume_music(1.0f), _debug(false), _loop(false) {}

void IMixer::init(const bool nosound, const bool nomusic) {
	if (nosound && nomusic) {
		_nosound = _nomusic = true;
		return;
	}
	
	Config->get("engine.sound.debug", _debug, false);

	int rate;
	Config->get("engine.sound.sample-rate", rate, 22050);
	TRY {
		if(Mix_OpenAudio(rate, AUDIO_S16LSB, 2, 1024) == -1)
			throw_sdl(("Mix_OpenAudio"));
	} CATCH("init", {
		_nosound = _nomusic = true;
		return;
	});
	
	
	Config->get("engine.sound.volume.fx", _volume_fx, 1.0f);
	Config->get("engine.sound.volume.ambience", _volume_ambience, 0.5f);
	Config->get("engine.sound.volume.music", _volume_music, 1.0f);
	
	LOG_DEBUG(("volumes: music: %g, ambience: %g, fx: %g", _volume_music, _volume_ambience, _volume_fx));
	
	delete _ogg;
	_ogg = NULL;
	
	_nosound = nosound;
	_nomusic = nomusic;
	
	TRY {
		if (!nomusic)
			_ogg = new OggStream;
	} CATCH("music thread startup", { _nomusic = true; return; })

	TRY {
		if (!nosound)
			_ambient = new OggStream;
	} CATCH("ambient thread startup", {})
	
}

void IMixer::deinit() {
	LOG_DEBUG(("cleaning up mixer..."));	
	delete _ogg; 
	_ogg = NULL;
	delete _ambient; 
	_ambient = NULL;
	
	if (!_nosound) {
		LOG_DEBUG(("cleaning up sounds..."));
		std::for_each(_sounds.begin(), _sounds.end(), delete_ptr2<Sounds::value_type>());
		_sounds.clear();
	}
	
	Mix_CloseAudio();
	
	_nosound = true;
	_nomusic = true;
}

IMixer::~IMixer() {}

#include "scoped_ptr.h"

void IMixer::loadPlaylist(const std::string &file) {
	if (_nomusic) 
		return;
	
	TRY {
		scoped_ptr<mrt::BaseFile> f(Finder->get_file(file, "rt"));
		std::string line;
		while(f->readLine(line)) {
			mrt::trim(line);
			_playlist[line] = false;
		}
		f->close();
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

	std::string real_file = Finder->find("tunes/" + fname, false);
	if (real_file.empty())
		return false;

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
		mrt::Chunk data;
		int rate, channels;
		OggStream::decode(data, Finder->find("sounds/" + filename), rate, channels);
		LOG_DEBUG(("sample %s decoded. rate: %d, size: %u", filename.c_str(), rate, (unsigned)data.getSize()));
		_sounds[filename] = new Sample(data);
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

void IMixer::playSample(const Object *o, const std::string &name, const bool loop, const float gain) {
	if (_nosound || name.empty())
		return;
}

void IMixer::setFXVolume(const float volume) {
	if (volume < 0 || volume > 1) 
		throw_ex(("volume value %g is out of range [0-1]", volume));	

	//fixme! 
	_volume_fx = volume;
}

void IMixer::setMusicVolume(const float volume) {
	if (volume < 0 || volume > 1) 
		throw_ex(("volume value %g is out of range [0-1]", volume));	
	
	if (_ogg)
		_ogg->setVolume(volume);

	_volume_music = volume;	
}

void IMixer::setAmbienceVolume(const float volume) {
	if (volume < 0 || volume > 1) 
		throw_ex(("volume value %g is out of range [0-1]", volume));	
	
	if (_ambient)
		_ambient->setVolume(volume);

	_volume_ambience = volume;	
}


void IMixer::updateObject(const Object *o) {
	//fixme
}

void IMixer::tick(const float dt) {
	if (_ogg != NULL && _ogg->idle()) {
		//LOG_DEBUG(("sound thread idle"));
		play();
	}
}


void IMixer::setListener(const v3<float> &pos, const v3<float> &vel, const float r) {
	//fixme
}
	



void IMixer::cancelSample(const Object *o, const std::string &name) {
	if (_nosound || name.empty())
		return;
}

void IMixer::cancelAll(const Object *o) {
	if (_nosound)
		return;
}


void IMixer::cancelAll() {
	stopAmbient();
	
	if (_nosound)
		return;

	//fixme
}

void IMixer::startAmbient(const std::string &fname) {
	if (_nosound) 
		return;
	TRY {
		if (_ambient)
			_ambient->play(Finder->find("sounds/ambient/" + fname), true, _volume_ambience);
	} CATCH("startAmbient", {});
}

void IMixer::stopAmbient() {
	if (_ambient == NULL) 
		return;
	_ambient->stop();
}
