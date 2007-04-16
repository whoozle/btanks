#ifndef __MRT_FS_NODE_H__
#define __MRT_FS_NODE_H__

#include <string>
#include "export_mrt.h"

namespace mrt {

class MRTAPI FSNode {
public:
static const bool exists(const std::string &fname);
};

}

#endif
