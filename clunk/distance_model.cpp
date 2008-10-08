#include "distance_model.h"
#include <math.h>

float DistanceModel::gain(float distance) {
	float gain = 0;
	switch(type) {
	case Inverse: 
		if (clamped) {
			if (distance < reference_distance)
				distance = reference_distance;
			if (distance > max_distance)
				distance = max_distance;
		}
		gain = reference_distance / (reference_distance + rolloff_factor * (distance - reference_distance));
		break;
		
	case Linear: 
		if (clamped && distance < reference_distance) {
			distance = reference_distance;
		} 
		
		if (distance > max_distance)
			distance = max_distance;
			
		gain = 1 - rolloff_factor * (distance - reference_distance) / (max_distance - reference_distance);
		break;
	
	case Exponent: 
		if (clamped) {
			if (distance < reference_distance)
				distance = reference_distance;
			if (distance > max_distance)
				distance = max_distance;
		}
		gain = powf(distance / reference_distance, - rolloff_factor);
		
		break;	
	}

	if (gain < 0)
		gain = 0;
	else if (gain > 1)
		gain = 1;

	return gain;
}

