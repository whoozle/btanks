#ifndef BTANKS_EDITOR_OBJECT_PROPERTIES_DIALOG_H__
#define BTANKS_EDITOR_OBJECT_PROPERTIES_DIALOG_H__

#include "menu/container.h"
#include <string>
#include <set>

class NumericControl;
class PopupMenu;
class Object;

class ObjectPropertiesDialog : public Container {
public: 
	const Object *object;
	ObjectPropertiesDialog(const int w);

	virtual bool onKey(const SDL_keysym sym); 

	void show(const Object *o, const std::set<std::string> &variants);
	void reset();

	void get(std::set<std::string> &labels) const;
	const int getZ() const;

	virtual void tick(const float dt);
private: 
	NumericControl *_z;
	PopupMenu *_menu;
};

#endif
