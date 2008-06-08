#ifndef BTANKS_EDITOR_ADD_OBJECT_DIALOG_H__
#define BTANKS_EDITOR_ADD_OBJECT_DIALOG_H__

#include "menu/scroll_list.h"
#include "mrt/xml.h"
#include <set>
#include <string>

class ObjectPropertiesDialog;

class AddObjectDialog : public ScrollList, private mrt::XMLParser {
public: 
	AddObjectDialog(const int w, const int h);
	~AddObjectDialog();
	virtual bool onKey(const SDL_keysym sym);
	virtual void tick(const float dt);
	
	const bool get(std::string &classname, std::string &animation, int &z);
	
	void get_variants(std::set<std::string> &variants, const std::string &classname) const;

private: 
	virtual void start(const std::string &name, Attrs &attr);
	virtual void end(const std::string &name);

	// temp parser objects 
	std::string _classname;
	//end of parser objects

	std::string _base, _fname;
	
	typedef std::set<std::string> Classes; 
	typedef std::multimap<const std::string, std::string> Animations, Variants;
	typedef std::vector<Animations::const_iterator> UserData;
	
	Classes _classes; 
	Animations _animations;
	Variants _variants;
	UserData _user_data;
	
	std::string selected_classname, selected_animation;
	int selected_z;
	
	ObjectPropertiesDialog *_z;
};

#endif

