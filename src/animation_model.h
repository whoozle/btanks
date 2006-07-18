#ifndef __BTANKS_ANIMATION_MODEL_H__
#define __BTANKS_ANIMATION_MODEL_H__

#include <vector>
#include <string>
#include <map>

struct Pose {
	Pose(const float speed) : speed(speed) {}

	const float speed;
	std::vector<unsigned int> frames;
};


class AnimationModel {
public:
	const float default_speed;
	AnimationModel(const float default_speed);
	
	void addPose(const std::string &id, Pose *pose);
	const Pose * getPose(const std::string &id) const;

private:
	typedef std::map<const std::string, Pose *> PoseMap;
	PoseMap _poses;
};

#endif
