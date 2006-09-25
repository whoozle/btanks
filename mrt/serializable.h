#ifndef __BTANKS_SERIALIZABLE_H__
#define __BTANKS_SERIALIZABLE_H__

namespace mrt {
class Serializator;
class Chunk;

class Serializable {
public:
	virtual void serialize(Serializator &s) const = 0;
	virtual void deserialize(const Serializator &s) = 0;

	void serialize2(mrt::Chunk &s) const;
	void deserialize2(const mrt::Chunk &s);

	virtual ~Serializable() {}
};
}

#endif

