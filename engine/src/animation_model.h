#ifndef __BTANKS_ANIMATION_MODEL_H__
#define __BTANKS_ANIMATION_MODEL_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/

#include <vector>
#include <string>
#include <map>
#include "export_btanks.h"

class BTANKSAPI Pose {
public:
	Pose(const float speed, const int z, const std::string &sound) : 
		speed(speed), z(z), sound(sound), gain(1.0f), frames(), need_notify(false) {}

	const float speed;
	int z;
	std::string sound;
	float gain;
	std::vector<unsigned int> frames;
	bool need_notify;
};


class BTANKSAPI AnimationModel {
public:
	const float default_speed;
	AnimationModel(const float default_speed);
	
	void addPose(const std::string &id, Pose *pose);
	const Pose * getPose(const std::string &id) const;
	~AnimationModel();

private:
	typedef std::map<const std::string, Pose *> PoseMap;
	PoseMap _poses;
};

class BTANKSAPI Animation {
public:
	std::string model, base_dir, surface;
	int tw, th;
	
	Animation(const std::string & model, const std::string &base_dir, const std::string &tile, const int tw, const int th);
};


#endif
