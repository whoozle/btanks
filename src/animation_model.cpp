#include "animation_model.h"
#include "mrt/logger.h"

AnimationModel::AnimationModel(const float default_speed) : default_speed(default_speed) {}

void AnimationModel::addPose(const std::string &id, Pose *pose) {
	delete _poses[id];
	_poses[id] = pose;
	LOG_DEBUG(("pose '%s' with %d frames added (speed: %f)", id.c_str(), pose->frames.size(), pose->speed));
}

