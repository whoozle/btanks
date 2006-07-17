#ifndef __BT_TILEMANAGER_H__
#define __BT_TILEMANAGER_H__

#include "mrt/singleton.h"
#include "mrt/xml.h"
#include <map>

namespace sdlx {
	class Surface;
}

class AnimatedObject;

class IResourceManager : public mrt::XMLParser {
public:
	IResourceManager();
	~IResourceManager();
	DECLARE_SINGLETON(IResourceManager);
	
	void init(const std::string &fname);
	void clear();
	
	AnimatedObject *getAnimation(const std::string &id);

private:
	virtual void start(const std::string &name, Attrs &attr);
	virtual void end(const std::string &name);
	virtual void charData(const std::string &data);
	
	typedef std::map<const std::string, AnimatedObject *> AnimationMap;
	AnimationMap _animations;
	
	long _tw, _th;
};

SINGLETON(ResourceManager, IResourceManager);

#endif

