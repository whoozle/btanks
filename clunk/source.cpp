#define _USE_MATH_DEFINES
#include <math.h>
#include "source.h"
#include <SDL.h>
#include "mrt/exception.h"
#include "mrt/chunk.h"
#include "sample.h"

using namespace clunk;
Source::Source(const Sample * sample, const bool loop, const v3<float> &delta, float gain, float pitch) : 
	sample(sample), loop(loop), delta_position(delta), gain(gain), pitch(pow(2.0f, pitch)), position(0) {}
	
void Source::idt(const v3<float> &delta, float &idt_offset, float &angle_gr) {
	float head_r = 0.09554140127388535032f;

	float direction = M_PI_2;
	float angle = direction - atan2f(delta.y, delta.x);
	
	angle_gr = angle * 180 / M_PI;
	while (angle_gr < 0)
		angle_gr += 360;
	
	//LOG_DEBUG(("relative position = (%g,%g,%g), angle = %g (%g)", delta.x, delta.y, delta.z, angle, angle_gr));
	
	float idt_angle = angle;
	while (idt_angle < 0)
		idt_angle += 2 * M_PI;
	while(idt_angle > 2 * M_PI)
		idt_angle -= 2 * M_PI;

	if (idt_angle >= M_PI_2 && idt_angle < M_PI) {
		idt_angle = M_PI - idt_angle;
	} else if (idt_angle >= M_PI && idt_angle < 3 * M_PI_2) {
		idt_angle = M_PI - idt_angle;
	} else if (idt_angle >= 3 * M_PI_2) {
		idt_angle -= M_PI * 2;
	}

	//printf("idt_angle = %g (%d)\n", idt_angle, (int)(idt_angle * 180 / M_PI));
	idt_offset = head_r / 343 * (idt_angle + sin(idt_angle));
	//printf("idt_offset %g", idt_offset);
}

#define WINDOW_SIZE 512

void Source::hrtf(mrt::Chunk &data, unsigned dst_n, const Sint16 *src, unsigned src_ch, unsigned src_n, const v3<float> &delta_position) {
	assert(position >= 0 && position < (int)src_n && dst_n != 0);
	unsigned n = (dst_n - 1) / WINDOW_SIZE + 1;
	LOG_DEBUG(("fft windows: %u", n));
}

float Source::process(mrt::Chunk &buffer, unsigned dst_ch, const v3<float> &delta_position) {
	LOG_DEBUG(("delta position: %g %g", delta_position.x, delta_position.y));
	
	kemar_ptr kemar_data;
	int angles;
	get_kemar_data(kemar_data, angles, delta_position);
	LOG_DEBUG(("data: %p, angles: %d", (void *) kemar_data, angles));
	
	float r2 = delta_position.quick_length();
	if (r2 < 1)
		r2 = 1;
	float vol = gain / r2;
	if (vol < 0)
		return 0;
	if (vol > 1)
		vol = 1;
	
	const Sint16 * src = (Sint16*) sample->data_ptr;
	if (src == NULL)
		throw_ex(("uninitialized sample used (%p)", (void *)sample));
	if (pitch < 0)
		throw_ex(("pitch %g could not be negative", pitch));
		

	unsigned src_ch = sample->spec.channels; 
	unsigned src_n = sample->data_len / src_ch / 2;
	if (position >= (int)src_n) {
		return 0;
	}

	Sint16 * dst = (Sint16*) buffer.getPtr();
	unsigned dst_n = buffer.getSize() / dst_ch / 2;

	mrt::Chunk sample3d;
	hrtf(sample3d, dst_n, src, src_ch, src_n, delta_position);
	
	float t_idt, angle_gr;
	idt(delta_position, t_idt, angle_gr);
	int idt_offset = (int)(t_idt * sample->spec.freq);
	//LOG_DEBUG(("idt offset %d samples", idt_offset));
	
	for(unsigned i = 0; i < dst_n; ++i) {
		for(unsigned c = 0; c < dst_ch; ++c) {
			int p = position + (int)(i * pitch);
			
			Sint16 v = 0;
			if (c <= 1) {
				bool left = c == 0;
				if (!left && idt_offset > 0) {
					p -= idt_offset;
				} else if (left && idt_offset < 0) {
					p += idt_offset;
				}
				
				if (loop || p >= 0 || p < (int)src_n) {
					p %= src_n;
					if (p < 0)	
						p += src_n;
	
					v = src[p * src_ch]; //always first channel, 3d sounds must be mono. fixme :)
				}
			}
			dst[i * dst_ch + c] = v;
		}
	}
	position += ((int)(dst_n * pitch));
	if (loop) {
		position %= src_n;
		//LOG_DEBUG(("position %d", position));
		if (position < 0)
			position += src_n;
	}
	
	return vol;
}

#include "kemar.h"

void Source::get_kemar_data(kemar_ptr & kemar_data, int & elev_n, const v3<float> &pos) {
	
	int elev_gr = (int)(180 * atan2(pos.z, sqrt(pos.x * pos.x + pos.y * pos.y)) / M_PI);

	kemar_data = NULL;
	elev_n = 0;
	if (pos.is0())
		return;

	if (elev_gr < -35) {
		kemar_data = elev_m40;
		elev_n = ELEV_M40_N;
	} else if (elev_gr < -25) {
		kemar_data = elev_m30;
		elev_n = ELEV_M30_N;
	} else if (elev_gr < -15) {
		kemar_data = elev_m20;
		elev_n = ELEV_M20_N;
	} else if (elev_gr < -5) {
		kemar_data = elev_m10;
		elev_n = ELEV_M10_N;
	} else if (elev_gr < 5) {
		kemar_data = elev_0;
		elev_n = ELEV_0_N;
	} else if (elev_gr < 15) {
		kemar_data = elev_10;
		elev_n = ELEV_10_N;
	} else if (elev_gr < 25) {
		kemar_data = elev_20;
		elev_n = ELEV_20_N;
	} else if (elev_gr < 35) {
		kemar_data = elev_30;
		elev_n = ELEV_30_N;
	} else if (elev_gr < 45) {
		kemar_data = elev_40;
		elev_n = ELEV_40_N;
	} else if (elev_gr < 55) {
		kemar_data = elev_50;
		elev_n = ELEV_50_N;
	} else if (elev_gr < 65) {
		kemar_data = elev_60;
		elev_n = ELEV_60_N;
	} else if (elev_gr < 75) {
		kemar_data = elev_70;
		elev_n = ELEV_70_N;
	} else if (elev_gr < 85) {
		kemar_data = elev_80;
		elev_n = ELEV_80_N;
	} else {
		kemar_data = elev_90;
		elev_n = ELEV_90_N;
	}
}
