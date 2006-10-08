#ifndef __BTANKS_MIXER_H__
#define __BTANKS_MIXER_H__

#include "mrt/singleton.h"
#include <string>
#include <map>
#include <AL/al.h>
#include <math/v3.h>

namespace mrt{
class Chunk;
}

class OggStream;
class Sample;
class Object;

class IMixer {
public:
	DECLARE_SINGLETON(IMixer);
	void init(const bool no_sound, const bool no_music);
	void loadPlaylist(const std::string &file);
	void play();
	
	//sample part
	void setListener(const v3<float> &pos, const v3<float> &vel);
	
	void loadSample(const std::string &filename);
	void playSample(const Object *o, const std::string &name, const bool loop);
	void cancelSample(const Object *o, const std::string &name);
	void cancelAll(const Object *o);
	void cancelAll();
	
	void updateObjects();
	
	IMixer();
	~IMixer();
private: 
	bool _nosound, _nomusic;
	
	typedef std::map<const std::string, Sample *> Sounds;
	Sounds _sounds;
	
	typedef std::map<const int, ALuint> Sources;
	Sources _sources;

	typedef std::map<const std::string, bool> PlayList;
	PlayList _playlist;
	std::string _now_playing;
	OggStream * _ogg;
};

SINGLETON(Mixer, IMixer);

#endif
