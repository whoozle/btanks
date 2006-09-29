#ifndef __BTANKS_PROTOCOL_H__
#define __BTANKS_PROTOCOL_H__

#include <sys/types.h>
#include <map>
#include <string>
#include "mrt/serializable.h"
#include "mrt/chunk.h"

namespace mrt {
	class TCPSocket;
}

	
class Message : public mrt::Serializable {
public: 
	enum Type {
		None, Ping, Pang, Pong,
		ServerStatus,
		PlayerState,
		UpdatePlayers,
		UpdateWorld
	};
	
	Message();
	Message(const Type type);
	
	const char * getType() const;

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);
	
	void set(const std::string &key, const std::string &value);
	const std::string &get(const std::string &key) const;
	
	Type type;

	mrt::Chunk data;
private:
	void readMap(const mrt::Serializator &s);
	void writeMap(mrt::Serializator &s) const;

	typedef std::map<const std::string, std::string> AttrMap;
	AttrMap _attrs;
};

#endif

