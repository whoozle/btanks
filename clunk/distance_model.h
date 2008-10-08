#ifndef CLUNK_DISTANCE_MODEL_H__
#define CLUNK_DISTANCE_MODEL_H__

#include "export_clunk.h"

namespace clunk {

//!Distance model used in context.

struct DistanceModel {
	enum Type { Inverse, Linear, Exponent };

	//!Type of the distance model
	Type type;
	//!clamping means cutting off max_distance, you must set max_distance to some value then.
	bool clamped;
	
	//!minimal distance for distance model, default == 1
	float reference_distance;
	//!maximum distance for clamping distance models. MUST NOT BE DEFAULT
	float max_distance; 
	//!rolloff factor for distance models
	float rolloff_factor;
	
	/*!
		\brief constructor
		\param[in] type type of the distance model: inversed, linear or exponent.
		\param[in] clamped means that max_distance will be used.
		\param[in] max_distance maximum distance for the model.
	*/ 
	DistanceModel(Type type, bool clamped, float max_distance = 0): type(type), clamped(clamped), 
	reference_distance(1), max_distance(max_distance), rolloff_factor(1)
	{}
	
	//! Computes gain by distance. Return values is in [0-1] range.
	float gain(float distance);
};

}

#endif

