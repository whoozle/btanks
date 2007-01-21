#ifndef __MRT_FS_NODE_H__
#define __MRT_FS_NODE_H__

#include <string>

namespace mrt {

class FSNode {
public:
static const bool exists(const std::string &fname);
};

}

#endif
