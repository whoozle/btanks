#ifndef BT_ENGINE_VIDEO_CONTROL_H__
#define BT_ENGINE_VIDEO_CONTROL_H__

#include "control.h"
#include <string>

class VideoControl : public Control {
public: 
	VideoControl(const std::string &base, const std::string &name);
private: 
	std::string base, name;
};


#endif

