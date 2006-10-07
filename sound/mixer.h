#ifndef __BTANKS_MIXER_H__
#define __BTANKS_MIXER_H__

#include "mrt/singleton.h"
#include <string>
#include <map>

namespace mrt{
class Chunk;
}

class OggStream;
class IMixer {
public:
	DECLARE_SINGLETON(IMixer);
	void init();
	void loadPlaylist(const std::string &file);
	void play();
	
	//sample part
	void loadSample(const std::string &filename);
	
	IMixer();
	~IMixer();
private: 
	typedef std::map<const std::string, mrt::Chunk *> Sounds;
	Sounds _sounds;

	typedef std::map<const std::string, bool> PlayList;
	PlayList _playlist;
	std::string _now_playing;
	OggStream * _ogg;
};

SINGLETON(Mixer, IMixer);

#endif
