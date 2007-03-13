#include "object.h"
#include "resource_manager.h"
#include "sdlx/rect.h"
#include "mrt/random.h"
#include "zbox.h"
#include "world.h"

class Teleport : public Object {
public: 
	Teleport() : Object("teleport"), track(0) {
		impassability = -1;
		setZ(-1);
	}

	virtual void onSpawn();
	virtual Object * clone() const;

	virtual void emit(const std::string &event, Object * emitter = NULL);
	~Teleport();

	virtual void tick(const float dt) {
		Object::tick(dt);
		if (!track)
			return;
		
		const Object *o = World->getObjectByID(track);
		if (o == NULL) {
			track = 0;
			return;
		}
		
		v2<int> pos, tpos;
		getCenterPosition(pos);
		o->getCenterPosition(tpos);
		if (pos.quick_distance(tpos) >= size.x * size.y) {
			LOG_DEBUG(("dropped target %d", track));
			track = 0;
			need_sync = true;
		}
	}
	
	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(track);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(track);
	}	


private: 
	typedef std::multimap<const std::string, Teleport *> TeleportMap;
	static TeleportMap _teleports;
	int track;
};

Teleport::TeleportMap Teleport::_teleports;

void Teleport::emit(const std::string &event, Object * emitter) {
	if (event == "collision" && emitter != NULL) {
		v2<int> epos, pos;
		emitter->getCenterPosition(epos);
		getPosition(pos);

		if (emitter->getID() == track) {
			return;
		}

		int dx = (int)(size.x / 6), dy = (int)(size.y / 6);
		sdlx::Rect r(pos.x + dx, pos.y + dy, (int)size.x - dx, (int)size.y - dy);

		std::vector<Teleport *> teleports;
		if (r.in(epos.x, epos.y)) {
			//LOG_DEBUG(("collided: %s", emitter->animation.c_str()));
			for(TeleportMap::const_iterator i = _teleports.lower_bound(registered_name); i != _teleports.upper_bound(registered_name); ++i) {
				if (i->second != this)
					teleports.push_back(i->second);
			}
		}

		if (teleports.empty())
			return;

		Teleport *dst = teleports[teleports.size() == 1?0: mrt::random(teleports.size())];
		dst->getCenterPosition(emitter->_position);

		emitter->_position -= emitter->size / 2;
		//LOG_DEBUG(("dst z = %d, dst box base = %d", dst->getZ(), ZBox::getBoxBase(dst->getZ())));
		emitter->setZBox(ZBox::getBoxBase(dst->getZ()));
		dst->track = emitter->getID();
		dst->need_sync = true;
	} else Object::emit(event, emitter);
}

Teleport::~Teleport() {
	for(TeleportMap::iterator i = _teleports.begin(); i != _teleports.end(); ) {
		if (i->second == this)
			_teleports.erase(i++);
		else ++i;
	}
}

void Teleport::onSpawn() {
	play("main", true);
	_teleports.insert(TeleportMap::value_type(registered_name, this));
}

Object * Teleport::clone() const {
	return new Teleport(*this);
}

REGISTER_OBJECT("teleport-a", Teleport, ());
REGISTER_OBJECT("teleport-b", Teleport, ());
REGISTER_OBJECT("teleport-c", Teleport, ());
REGISTER_OBJECT("teleport-d", Teleport, ());

REGISTER_OBJECT("teleport-e", Teleport, ());
REGISTER_OBJECT("teleport-f", Teleport, ());
REGISTER_OBJECT("teleport-g", Teleport, ());
REGISTER_OBJECT("teleport-h", Teleport, ());

