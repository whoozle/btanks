#include "checkpoint.h"

Checkpoint::Checkpoint(const ZBox & zbox, const std::string &name) : ZBox(zbox), _name(name) {}

const bool Checkpoint::final() const {
	return _name.substr(0, 6) == "final:";
}
