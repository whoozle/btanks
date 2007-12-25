#ifndef BTANKS_NET_SCANNER_H__
#define BTANKS_NET_SCANNER_H__

#include "sdlx/thread.h"

class Scanner : public sdlx::Thread {
public:
	Scanner(); 
	~Scanner();
private: 
	virtual const int run();
	volatile bool _running, _scan;
};


#endif
