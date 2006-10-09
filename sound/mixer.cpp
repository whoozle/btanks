#include "mixer.h"
#include "mrt/file.h"
#include "mrt/exception.h"
#include "mrt/random.h"
#include "mrt/chunk.h"
#include <assert.h>
#include "utils.h"

#include "ogg_stream.h"
#include "al_ex.h"
#include "sample.h"
#include <AL/alut.h>

#include "config.h"
#include "world.h"
#include "object.h"
#include "math/v3.h"


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
	_nomusic = true;
	
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
		_nomusic = true;
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
	LOG_DEBUG(("loading sample %s", filename.c_str()));
	if (_sounds.find(filename) != _sounds.end()) {
		LOG_DEBUG(("already loaded, skipped."));
		return;
	}

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

void IMixer::playSample(const Object *o, const std::string &name, const bool loop) {
	if (_nosound || name.empty())
		return;
	const int id = o->getID();
	LOG_DEBUG(("object: %d requests %s (%s)", id, name.c_str(), loop?"loop":"single"));
	Sounds::const_iterator i = _sounds.find(name);
	if (i == _sounds.end()) {
		LOG_WARN(("sound %s was not loaded. skipped.", name.c_str()));
		return;
	}
	const Sample &sample = *i->second;
	
	ALuint source;
	alGenSources(1, &source);
	AL_CHECK(("alGenSources"));
	_sources.insert(Sources::value_type(id, source));

	v3<float> pos, vel;
	o->getInfo(pos, vel);

	GET_CONFIG_VALUE("engine.sound.positioning-divisor", float, k, 200.0);
	
	ALfloat al_pos[] = { pos.x / k, -pos.y / k, 0*pos.z / k };
	ALfloat al_vel[] = { vel.x / k, -vel.y / k, 0*vel.z / k };
	
	alSourcei (source, AL_BUFFER,   sample.buffer);
	alSourcef (source, AL_PITCH,    1.0          );
	alSourcef (source, AL_GAIN,     1.0          );
	alSourcefv(source, AL_POSITION, al_pos       );
	alSourcefv(source, AL_VELOCITY, al_vel       );
	alSourcei (source, AL_LOOPING,  loop?AL_TRUE:AL_FALSE );
	alSourcePlay(source);
}

void IMixer::updateObjects() {
	if (_nosound) 
		return;
		
	for(Sources::iterator j = _sources.begin(); j != _sources.end();) {
	int last_id = -1;
	TRY {
		ALuint source = j->second;
		ALenum state;
		alGetSourcei(source, AL_SOURCE_STATE, &state);

		if (state != AL_PLAYING) {
			alDeleteSources(1, &source);
			_sources.erase(j++);
			continue;
		}
		if (j->first == last_id) {
			++j;
			continue;
		}

		last_id = j->first;
		Object *o = World->getObjectByID(j->first);
		
		v3<float> pos, vel;
		o->getInfo(pos, vel);
		GET_CONFIG_VALUE("engine.sound.positioning-divisor", float, k, 200.0);
		
		ALfloat al_pos[] = { pos.x / k, -pos.y / k, 0*pos.z / k };
		ALfloat al_vel[] = { vel.x / k, -vel.y / k, 0*vel.z / k };
	
		alSourcefv(j->second, AL_POSITION, al_pos);
		alSourcefv(j->second, AL_VELOCITY, al_vel);
		++j;
	} CATCH("updateObjects", {++j; continue;})
	}
}


void IMixer::setListener(const v3<float> &pos, const v3<float> &vel) {
	GET_CONFIG_VALUE("engine.sound.positioning-divisor", float, k, 200.0);
		
	ALfloat al_pos[] = { pos.x / k, -pos.y / k, pos.z / k };
	ALfloat al_vel[] = { vel.x / k, -vel.y / k, vel.z / k };
		
	alListenerfv(AL_POSITION,    al_pos);
	alListenerfv(AL_VELOCITY,    al_vel);
	//alListenerfv(AL_ORIENTATION, al_vel);
}
	



void IMixer::cancelSample(const Object *o, const std::string &name) {
	if (_nosound || name.empty())
		return;
/*	LOG_DEBUG(("object %d cancels %s", o->getID(), name.c_str()));
	Sources::iterator j = _sources.find(o->getID());
	if (j == _sources.end())
		return;
	alSourceStop(j->second);
*/}

void IMixer::cancelAll(const Object *o) {
	if (_nosound)
		return;
	
/*	Sources::iterator j = _sources.find(o->getID());
	if (j == _sources.end())
		return;
	alSourceStop(j->second);
*/}


void IMixer::cancelAll() {
	if (_nosound)
		return;
	LOG_DEBUG(("stop playing anything"));
	for(Sources::iterator j = _sources.begin(); j != _sources.end(); ++j) {
		alSourceStop(j->second);
		alDeleteSources(1, &j->second);
	}
	_sources.clear();
}
