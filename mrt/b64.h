#ifndef __MRT_B64_H__
#define __MRT_B64_H__

#include <string>

namespace mrt {

class Chunk;

class Base64 {
public:
	//static encode(std::string &dst, const mrt::chunk &src);
	static void decode(mrt::Chunk &dst, const std::string &src);
};

}

#endif

