#ifndef CLUNK_CONTEXT_H__
#define CLUNK_CONTEXT_H__

#include "export_clunk.h"

namespace clunk {
class CLUNKAPI Context {
public: 
	Context();
	
	void init(const int sample_rate, const int channels, int period_size);
	void deinit();
	
	~Context();
private: 
	int sample_rate, channels, period_size;
};
}


#endif

