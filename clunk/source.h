#ifndef CLUNK_SOURCE_H__
#define CLUNK_SOURCE_H__

/* libclunk - realtime 2d/3d sound render library
 * Copyright (C) 2005-2008 Netive Media Group
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "export_clunk.h"
#include "v3.h"
#include <SDL_audio.h>

namespace mrt {
	class Chunk;
}

namespace clunk {

#ifndef CLUNK_WINDOW_SIZE
#	define CLUNK_WINDOW_SIZE 512
#endif

#ifndef CLUNK_WINDOW_OVERLAP
#	define CLUNK_WINDOW_OVERLAP 64
#endif

class Sample;
class CLUNKAPI Source {
public:
	const Sample * const sample;

	bool loop;
	v3<float> delta_position; //0 - from the center of the object. 
	float gain;
	float pitch;

	float reference_distance, rolloff_factor;
	
	Source(const Sample * sample, const bool loop = false, const v3<float> &delta = v3<float>(), float gain = 1, float pitch = 1);

	float process(mrt::Chunk &buffer, unsigned ch, const v3<float> &position, const float fx_volume);
	bool playing() const;

	void update_position(const int dp);
	
private: 
	typedef const float (*kemar_ptr)[2][512];
	void get_kemar_data(kemar_ptr & kemar_data, int & samples, const v3<float> &delta_position);

	void idt(const v3<float> &delta, float &idt_offset, float &angle_gr);
	void hrtf(mrt::Chunk &result, int dst_n, const Sint16 *src, int src_ch, int src_n, const kemar_ptr& kemar_data, int kemar_idx);

	int position;
	
	Sint16 overlap_data[CLUNK_WINDOW_OVERLAP];
	bool use_overlap;
};
}

#endif
