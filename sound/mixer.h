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
	
	//sample part
	void setListener(const v3<float> &pos, const v3<float> &vel);
	
	void loadSample(const std::string &filename, const std::string &classname = std::string());
	void playSample(const Object *o, const std::string &name, const bool loop);
	void playRandomSample(const Object *o, const std::string &classname, const bool loop);
	void cancelSample(const Object *o, const std::string &name);
	void cancelAll(const Object *o);
	void cancelAll();
	
	void tick(const float dt);
	void updateObject(const Object *o);
	
	void setFXVolume(const float volume);
	void setMusicVolume(const float volume);
	
	IMixer();
	~IMixer();
private: 
	bool _nosound, _nomusic;
	Alarm _update_objects;
	
	typedef std::map<const std::string, Sample *> Sounds;
	Sounds _sounds;
	
	typedef std::multimap<const std::pair<int, std::string> , ALuint> Sources;
	Sources _sources;
	
	typedef std::map<const std::string, std::set<std::string> > Classes;
	Classes _classes;

	typedef std::map<const std::string, bool> PlayList;
	PlayList _playlist;
	std::string _now_playing;
	OggStream * _ogg;
	
	float _volume_fx, _volume_music;
	
	IMixer(const IMixer &);
	const IMixer& operator=(const IMixer &);
};

SINGLETON(BTANKSAPI, Mixer, IMixer);

#endif
