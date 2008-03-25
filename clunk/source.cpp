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


#define _USE_MATH_DEFINES
#include <math.h>
#include "source.h"
#include <SDL.h>
#include "mrt/exception.h"
#include "mrt/chunk.h"
#include "sample.h"

#ifdef _WINDOWS
#	define pow10f(x) powf(10.0f, (x))
#	define log2f(x) (logf(x) / M_LN2)
#endif

using namespace clunk;
Source::Source(const Sample * sample, const bool loop, const v3<float> &delta, float gain, float pitch) : 
	sample(sample), loop(loop), delta_position(delta), gain(gain), pitch(pitch), 
	reference_distance(1), rolloff_factor(1), 
	position(0)  {
	if (sample == NULL)
		throw_ex(("sample for source cannot be NULL"));
}
	
bool Source::playing() const {
	return loop?true: position < (int)(sample->data.getSize() / sample->spec.channels / 2);
}
	
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
	idt_offset = - head_r / 343 * (idt_angle + sin(idt_angle));
	//printf("idt_offset %g", idt_offset);
}

#define WINDOW_SIZE 512
#include "kiss/kiss_fftr.h"

void Source::hrtf(mrt::Chunk &result, int dst_n, const Sint16 *src, int src_ch, int src_n, const kemar_ptr& kemar_data, int kemar_idx) {
	kiss_fftr_cfg kiss_cfg = kiss_fftr_alloc(512, 0, NULL, NULL);
	kiss_fftr_cfg kiss_cfg_i = kiss_fftr_alloc(512, 1, NULL, NULL);
	
	int n = (dst_n - 1) / WINDOW_SIZE + 1;
	result.setSize(2 * WINDOW_SIZE * n);
	Sint16 *dst = (Sint16 *)result.getPtr();
	
	for(int i = 0; i < n; ++i) {
		kiss_fft_scalar src_data[WINDOW_SIZE];
		kiss_fft_cpx freq[WINDOW_SIZE / 2 + 1];
		//printf("fft #%d\n", i);
		for(int j = 0; j < WINDOW_SIZE; ++j) {
			int p = (int)(position + i * WINDOW_SIZE + j * pitch);

			int v = 0;
			if (p >= 0 || p < src_n || loop) {
				p %= src_n;
				if (p < 0)
					p += src_n;
				v = src[p * src_ch];
			}
			src_data[j] = v / 32767.0;
		}
		
		kiss_fftr(kiss_cfg, src_data, freq);
		
		//LOG_DEBUG(("kemar angle index: %d\n", kemar_idx));
		for(int j = 0; j <= WINDOW_SIZE / 2; ++j) {
			//float * dst = (ch == 0)?tr_left + pos:tr_right + pos;
			float len = sqrt(freq[j].r * freq[j].r + freq[j].i * freq[j].i);
			//LOG_DEBUG(("length: %g", len));
			float m = sqrt(pow10f(kemar_data[kemar_idx][1][j] * len / 20));
			//printf("%d: multiplicator = %g, len: %g\n", j, m, len);
			freq[j].r *= m;
			freq[j].i *= m;
			//printf("%g <--> %g\n", kemar_data[idx][0][j], elev_0[idx][1][j]);
		}

		float max = 32766;
		kiss_fftri(kiss_cfg_i, freq, src_data);
		for(int j = 0; j < WINDOW_SIZE; ++j) {
			int x = (int)(src_data[j] * (max / WINDOW_SIZE));
			if (x > 32766) {
				max /= x / 32766.0;
				x = (int)(src_data[j] * (max / WINDOW_SIZE));
			}
			dst[i * WINDOW_SIZE + j] = x;
			//LOG_DEBUG(("%g: %d", src_data[j], dst[i * WINDOW_SIZE + j]));
			//printf("%g,%g ", tr[pos + j], tr[pos + j] / 1024);
		}
	}
	kiss_fft_free(kiss_cfg);
	kiss_fft_free(kiss_cfg_i);
}

void Source::update_position(const int dp, const int src_n) {
	position += dp;
	if (loop) {
		position %= src_n;
		//LOG_DEBUG(("position %d", position));
		if (position < 0)
			position += src_n;
	}
}


float Source::process(mrt::Chunk &buffer, unsigned dst_ch, const v3<float> &delta_position, const float fx_volume) {
	Sint16 * dst = (Sint16*) buffer.getPtr();
	unsigned dst_n = buffer.getSize() / dst_ch / 2;
	const Sint16 * src = (Sint16*) sample->data.getPtr();
	if (src == NULL)
		throw_ex(("uninitialized sample used (%p)", (void *)sample));
	if (pitch < 0)
		throw_ex(("pitch %g could not be negative", pitch));
		
	unsigned src_ch = sample->spec.channels; 
	unsigned src_n = sample->data.getSize() / src_ch / 2;

	//LOG_DEBUG(("delta position: %g %g", delta_position.x, delta_position.y));
	float r2 = delta_position.length();
	if (r2 < reference_distance) 
		r2 = reference_distance;
	
	//float vol = (r2 > 1)?fx_volume * gain / r2: fx_volume * gain;
	float vol = fx_volume * gain / (1 + rolloff_factor * ((r2 - reference_distance) / reference_distance));
	
	if (vol > 1)
		vol = 1;

	if (vol < 0 || (int)floor(SDL_MIX_MAXVOLUME * vol + 0.5f) <= 0) {
		update_position((int)(dst_n * pitch), src_n);
		return 0;
	}
	
	kemar_ptr kemar_data;
	int angles;
	get_kemar_data(kemar_data, angles, delta_position);

	if (r2 < 1 || kemar_data == NULL) {
		//2d stereo sound! 
		for(unsigned i = 0; i < dst_n; ++i) {
			for(unsigned c = 0; c < dst_ch; ++c) {
				int p = position + (int)(i * pitch);
			
				Sint16 v = 0;
				if (p >= 0 && p < (int)src_n) {
					if (c < src_ch) {
						v = src[p * src_ch + c];
					} else {
						v = src[p * src_ch];//expand mono channel if needed
					}
				}
				dst[i * dst_ch + c] = v;
			}
		}
		update_position((int)(dst_n * pitch), src_n);
		return vol;
	}
	
	//LOG_DEBUG(("data: %p, angles: %d", (void *) kemar_data, angles));
	update_position(0, src_n);
	
	if (position >= (int)src_n) {
		LOG_WARN(("process called on inactive source"));
		return 0;
	}

	float t_idt, angle_gr;
	idt(delta_position, t_idt, angle_gr);

	const int kemar_idx_right = ((((int)angle_gr  + 180 / (int)angles)/ (360 / (int)angles)) % (int)angles);
	const int kemar_idx_left = (((360 - (int)angle_gr  + 180 / (int)angles)/ (360 / (int)angles)) % (int)angles);
	
	int idt_offset = (int)(t_idt * sample->spec.freq);

	mrt::Chunk sample3d_left, sample3d_right;
	int idt_abs = idt_offset > 0? idt_offset: -idt_offset;
	hrtf(sample3d_left, (int)((dst_n + idt_abs) * pitch), src, src_ch, src_n, kemar_data, kemar_idx_left);
	hrtf(sample3d_right, (int)((dst_n + idt_abs) * pitch), src, src_ch, src_n, kemar_data, kemar_idx_right);
	
	//LOG_DEBUG(("angle: %g", angle_gr));
	//LOG_DEBUG(("idt offset %d samples", idt_offset));
	Sint16 * src_3d[2] = { (Sint16 *)sample3d_left.getPtr(), (Sint16 *)sample3d_right.getPtr() };
	
	for(unsigned i = 0; i < dst_n; ++i) {
		for(unsigned c = 0; c < dst_ch; ++c) {
			int p = idt_abs + (int)(i * pitch);
			
			Sint16 v = 0;
			if (c <= 1) {
				bool left = c == 0;
				if (!left && idt_offset > 0) {
					p -= idt_offset;
				} else if (left && idt_offset < 0) {
					p += idt_offset;
				}
				if (p >= 0 && p * 2 < (int)sample3d_left.getSize())
					v = src_3d[c][p];
				//LOG_DEBUG(("%d->%d", p, v));
			}
			dst[i * dst_ch + c] = v;
		}
	}
	update_position((int)(dst_n * pitch), src_n);	
	return vol;
}

#include "kemar.h"

void Source::get_kemar_data(kemar_ptr & kemar_data, int & elev_n, const v3<float> &pos) {
	
	kemar_data = NULL;
	elev_n = 0;
	if (pos.is0())
		return;

	int elev_gr = (int)(180 * atan2(pos.z, sqrt(pos.x * pos.x + pos.y * pos.y)) / M_PI);

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
