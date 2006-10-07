#include "mixer.h"
#include "mrt/file.h"
#include "mrt/exception.h"
#include "mrt/random.h"
#include <assert.h>

#include "ogg_stream.h"
#include <AL/alut.h>

#define AL_CHECK(fmt) if (alGetError() != AL_NO_ERROR) \
	throw_ex(fmt)

IMPLEMENT_SINGLETON(Mixer, IMixer)

IMixer::IMixer() : _ogg(NULL) {}

void IMixer::init() {
	delete _ogg;
	_ogg = NULL;
	
	alutInit(NULL, NULL);
	AL_CHECK(("alutInit"));
}

IMixer::~IMixer() {
	delete _ogg; 
	_ogg = NULL;
	alutExit();
}


void IMixer::loadPlaylist(const std::string &file) {
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
	LOG_DEBUG(("loaded %d songs in playlist", _playlist.size()));
}

void IMixer::play() {
	int n = _playlist.size();
	if (n == 0) {
		LOG_WARN(("nothing to play"));
		return;
	}
	int p = mrt::random(n);
	
	PlayList::iterator i = _playlist.begin();
	while(p--) ++i;
	assert(i != _playlist.end());
	
	const std::string fname = i->first;
	LOG_DEBUG(("playing %s",fname.c_str()));
	std::string::size_type dp = fname.rfind('.');
	std::string ext = "unknown";
	if (dp != std::string::npos)
		ext = fname.substr(dp + 1);
	
	if (ext != "ogg") {
		LOG_WARN(("cannot play non-ogg files(%s). fixme.", ext.c_str()));
		return;
	}

	i->second = true;
	if (_ogg == NULL) 
		_ogg = new OggStream;

	_ogg->open(fname);
}
