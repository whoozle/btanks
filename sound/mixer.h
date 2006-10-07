#ifndef __BTANKS_MIXER_H__
#define __BTANKS_MIXER_H__

#include "mrt/singleton.h"
#include <string>
#include <map>

class OggStream;
class IMixer {
public:
	DECLARE_SINGLETON(IMixer);
	void init();
	void loadPlaylist(const std::string &file);
	void play();
	IMixer();
	~IMixer();
private: 
	typedef std::map<const std::string, bool> PlayList;
	PlayList _playlist;
	std::string _now_playing;
	OggStream * _ogg;
};

SINGLETON(Mixer, IMixer);

#endif
