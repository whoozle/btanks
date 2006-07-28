#ifndef __BTANKS_SERIALIZABLE_H__
#define __BTANKS_SERIALIZABLE_H__

namespace mrt {
class Serializator;
class Serializable {
public:
	virtual void serialize(Serializator &s) const = 0;
	virtual void deserialize(const Serializator &s) = 0;
	virtual ~Serializable() {}
};
}

#endif

