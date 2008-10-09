#ifndef __BTANKS_MIXER_H__
#define __BTANKS_MIXER_H__

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

#include "mrt/singleton.h"
#include <string>
#include <map>
#include <set>
#include <math/v3.h>
#include "alarm.h"
#include "sl08/sl08.h"

namespace mrt{
class Chunk;
}

namespace clunk {
class Context;
class Object;
class Source;
class Sample;
}

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
	void set_listener(const v3<float> &pos, const v3<float> &vel, const float r);
	void get_listener(v3<float> &pos, v3<float> &vel, float& r);
	
	void loadSample(const std::string &filename, const std::string &classname = std::string());
	void playSample(Object *o, const std::string &name, const bool loop, const float gain = 1.0);
	void playRandomSample(Object *o, const std::string &classname, const bool loop, const float gain = 1.0);
	
	void cancel_all();
	//void replace(const Object *old_object, const Object *new_object);
	
	void tick(const float dt);
	
	void setFXVolume(const float volume);
	void setAmbienceVolume(const float volume);
	void setMusicVolume(const float volume);
	
	IMixer();
	~IMixer();
	
	void startAmbient(const std::string &fname);
	void stopAmbient();

private:
	sl08::slot1<void, const Object *, IMixer> update_object_slot;
	sl08::slot1<void, const Object *, IMixer> delete_object_slot;
	//sl08::slot2<void, const Object *, const Object *, IMixer> replace_id_object_slot;

	bool _nosound, _nomusic;

	typedef std::map<const std::string, clunk::Sample *> Sounds;
	Sounds _sounds;
	
	typedef std::map<const std::string, std::set<std::string> > Classes;
	Classes _classes;

	typedef std::map<const std::string, bool> PlayList;
	PlayList _playlist;
	std::string _now_playing;
	
	float _volume_fx, _volume_ambience, _volume_music;
	
	bool _debug, _loop;
	
	
	clunk::Context *_context;
	
	v3<float> listener_pos, listener_vel;
	
	IMixer(const IMixer &);
	const IMixer& operator=(const IMixer &);
};

SINGLETON(BTANKSAPI, Mixer, IMixer);

#endif
