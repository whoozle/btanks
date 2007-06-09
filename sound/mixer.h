#ifndef __BTANKS_MIXER_H__
#define __BTANKS_MIXER_H__

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

#include "mrt/singleton.h"
#include <string>
#include <map>
#include <set>
#include <AL/al.h>
#include <AL/alc.h>
#include <math/v3.h>
#include "alarm.h"

namespace mrt{
class Chunk;
}

class OggStream;
class Sample;
class Object;

class BTANKSAPI IMixer {
public:
	DECLARE_SINGLETON(IMixer);
	void init(const bool no_sound, const bool no_music);
	void deinit();
	void loadPlaylist(const std::string &file);
	void play();
	const bool play(const std::string &fname, const bool continuous = false);
	void reset(); //stop playing custom music
	
	//sample part
	void setListener(const v3<float> &pos, const v3<float> &vel, const float r);
	
	void loadSample(const std::string &filename, const std::string &classname = std::string());
	void playSample(const Object *o, const std::string &name, const bool loop, const float gain = 1.0);
	void playRandomSample(const Object *o, const std::string &classname, const bool loop, const float gain = 1.0);
	void cancelSample(const Object *o, const std::string &name);
	void cancelAll(const Object *o);
	void cancelAll();
	
	void tick(const float dt);
	void updateObject(const Object *o);
	
	void setFXVolume(const float volume);
	void setMusicVolume(const float volume);
	
	IMixer();
	~IMixer();
	
	void startAmbient(const std::string &fname);
	void stopAmbient();

private:
	ALCdevice * alc_device;
	ALCcontext * alc_context;	
	
	void dumpContextAttrs() const;

	std::set<ALuint> _free_sources;
	bool _no_more_sources;
	
	const bool generateSource(ALuint &source);
	void deleteSource(const ALuint source);
	
	const unsigned purgeInactiveSources();

	bool _nosound, _nomusic;
	
	typedef std::map<const std::string, Sample *> Sounds;
	Sounds _sounds;
	
	struct SourceInfo {
		std::string name;
		bool loop;
		ALuint source;
		
		bool persistent;
		SourceInfo(const std::string &name, const bool loop, const ALuint source);
		
		const bool playing() const;
		
		v3<float> pos, vel;
		void updatePV();
	};
	
	typedef std::multimap<const int, SourceInfo> Sources;
	Sources _sources;
	
	typedef std::map<const std::string, std::set<std::string> > Classes;
	Classes _classes;

	typedef std::map<const std::string, bool> PlayList;
	PlayList _playlist;
	std::string _now_playing;
	OggStream * _ogg, *_ambient;
	ALuint _ogg_source, _ambient_source;
	
	float _volume_fx, _volume_music;
	
	bool _debug, _loop;
	
	IMixer(const IMixer &);
	const IMixer& operator=(const IMixer &);
};

SINGLETON(BTANKSAPI, Mixer, IMixer);

#endif
