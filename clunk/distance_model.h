#ifndef CLUNK_DISTANCE_MODEL_H__
#define CLUNK_DISTANCE_MODEL_H__

#include "export_clunk.h"

namespace clunk {

//!Distance model used in context.

struct DistanceModel {
	//!Type of the distance model: inversed, linear or exponent.
	enum Type { Inverse, Linear, Exponent };

	//!Type of the distance model
	Type type;
	//!Clamping means cutting off max_distance, you must set max_distance to some value then.
	bool clamped;
	
	//!Minimal distance for distance model, default == 1
	float reference_distance;
	//!Maximum distance for clamping distance models. MUST NOT BE DEFAULT
	float max_distance; 
	//!Rolloff factor for distance models
	float rolloff_factor;
	
	/*!
		\brief Constructor
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

