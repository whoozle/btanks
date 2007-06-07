#include "object.h"
#include "resource_manager.h"
#include "sdlx/rect.h"
#include "mrt/random.h"
#include "zbox.h"
#include "world.h"
#include "player_manager.h"
#include "player_slot.h"
#include "game_monitor.h"

class Teleport : public Object {
public: 
	Teleport() : Object("teleport"), track(0) {
		impassability = -1;
		hp = -1;
		pierceable = true;
		setZ(-1);
	}

	virtual void onSpawn();
	virtual Object * clone() const;

	virtual void emit(const std::string &event, Object * emitter = NULL);
	~Teleport();

	virtual void tick(const float dt);
	
	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(track);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(track);
		
		_teleports.insert(this);
	}


private: 
	typedef std::set<Teleport *> Teleports;
	static Teleports _teleports;
	int track;
};

Teleport::Teleports Teleport::_teleports;

void Teleport::tick(const float dt) {
	Object::tick(dt);
	if (GameMonitor->disabled(this)) {
		if (getState() != "hold") {
			cancelAll();
			play("hold", true);
		}
		return;
	}

		if (!track || PlayerManager->isClient())
			return;
		
		const Object *o = World->getObjectByID(track);
		if (o == NULL) {
			track = 0;
			need_sync = true;
			return;
		}
		PlayerSlot *slot = PlayerManager->getSlotByID(track);
		if (slot != NULL) {
			slot->dont_interpolate = true;
			slot->need_sync = true;
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


void Teleport::emit(const std::string &event, Object * emitter) {
	if (!PlayerManager->isClient() && event == "collision" && emitter != NULL) {
		if (getState() == "hold") {
			return;
		}
		
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
			for(Teleports::const_iterator i = _teleports.begin(); i != _teleports.end(); ++i) {
				if (*i != this && _variants.same((*i)->_variants))
					teleports.push_back(*i);
			}
		}

		if (teleports.empty())
			return;

		Teleport *dst = teleports[(teleports.size() == 1)?0: mrt::random(teleports.size())];
		dst->getCenterPosition(emitter->_position);

		emitter->_position -= emitter->size / 2;
		//LOG_DEBUG(("dst z = %d, dst box base = %d", dst->getZ(), ZBox::getBoxBase(dst->getZ())));
		emitter->setZBox(ZBox::getBoxBase(dst->getZ()));
		dst->track = emitter->getID();
		dst->need_sync = true;
		dst->playSound("teleport", false);
	} else Object::emit(event, emitter);
}

Teleport::~Teleport() {
	_teleports.erase(this);
}

void Teleport::onSpawn() {
	play("main", true);
	_teleports.insert(this);
}

Object * Teleport::clone() const {
	return new Teleport(*this);
}

REGISTER_OBJECT("teleport", Teleport, ());
