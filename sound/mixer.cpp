#include "mixer.h"
#include "mrt/file.h"
#include "mrt/exception.h"
#include "mrt/random.h"
#include "mrt/chunk.h"
#include <assert.h>
#include "utils.h"

#include "ogg_stream.h"
#include "sample.h"
#include <AL/alut.h>

#include "config.h"

#define AL_CHECK(fmt) if (alGetError() != AL_NO_ERROR) \
	throw_ex(fmt)

IMPLEMENT_SINGLETON(Mixer, IMixer)

IMixer::IMixer() : _nosound(true), _nomusic(true), _ogg(NULL) {}

void IMixer::init(const bool nosound, const bool nomusic) {
	if (nosound && nomusic) {
		_nosound = _nomusic = true;
		return;
	}
	
	delete _ogg;
	_ogg = NULL;
	
	alutInit(NULL, NULL);
	AL_CHECK(("alutInit"));
	
	TRY {
		GET_CONFIG_VALUE("engine.sound.doppler-factor", float, df, 1.0);
		alDopplerFactor(df);
		AL_CHECK(("alDopplerFactor"));
	
		GET_CONFIG_VALUE("engine.sound.doppler-velocity", float, dv, 1000);
		alDopplerVelocity(dv);
	} CATCH("init", {});
	_nosound = nosound;
	_nomusic = nomusic;
}

IMixer::~IMixer() {
	_nosound = true;
	
	delete _ogg; 
	_ogg = NULL;
	
	LOG_DEBUG(("cleaning up sounds..."));
	std::for_each(_sounds.begin(), _sounds.end(), delete_ptr2<Sounds::value_type>());
	_sounds.clear();
	LOG_DEBUG(("shutting down openAL.."));
	
	alutExit();
}


void IMixer::loadPlaylist(const std::string &file) {
	if (_nomusic) 
		return;
	
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
	if (_nomusic) 
		return;

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

void IMixer::loadSample(const std::string &filename) {
	if (_nosound) 
		return;

	Sample * sample = NULL;
	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");
	TRY {
		sample = new Sample;
		OggStream::decode(*sample, data_dir + "/sounds/" + filename);
		LOG_DEBUG(("sample %s decoded. rate: %u, size: %u", filename.c_str(), (unsigned)sample->rate, (unsigned)sample->data.getSize()));

		sample->init();
				
		_sounds[filename] = sample;
	} CATCH("loadSample", { delete sample; sample = NULL; });
}

void IMixer::playSample(const int id, const std::string &name, const bool loop) {
	if (_nosound || name.empty())
		return;
	LOG_DEBUG(("object: %d requests %s (%s)", id, name.c_str(), loop?"loop":"single"));
}

void IMixer::cancelSample(const int id, const std::string &name) {
	if (_nosound || name.empty())
		return;
	LOG_DEBUG(("object %d cancels %s", id, name.c_str()));
}

void IMixer::cancelAll(const int id) {
	if (_nosound)
		return;
}


void IMixer::cancelAll() {
	if (_nosound)
		return;
	LOG_DEBUG(("stop playing anything"));
}
