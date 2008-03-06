#ifndef CLUNK_OBJECT_H__
#define CLUNK_OBJECT_H__

#include "export_clunk.h"
#include "v3.h"

namespace clunk {
class Context;

class CLUNKAPI Object {
public: 
	~Object();
	void updatePV(const v3<float> &pos, const v3<float> &vel);

private: 
	friend class Context;
	Object(Context *context);
	Context *context;	
};
}

#endif

