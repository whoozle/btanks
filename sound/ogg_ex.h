#ifndef __BTANKS_OGG_EX_H__
#define __BTANKS_OGG_EX_H__

#include "mrt/exception.h"

class OggException : public mrt::Exception {
public:
	OggException(const int r) : _r(r) {}
	const std::string getCustomMessage();
	const int getCode() const throw() { return _r; }
	virtual ~OggException() throw() {}
private: 
	int _r;
};

#define throw_ogg(r, s) throw_generic_no_default(OggException, s, (r));

#endif

