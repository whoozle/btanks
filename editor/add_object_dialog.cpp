#include "add_object_dialog.h"
#include "mrt/file.h"
#include "mrt/directory.h"
#include "finder.h"
#include "mrt/exception.h"
#include "resource_manager.h"
#include <algorithm>
#include "object_properties.h"
#include "scoped_ptr.h"

AddObjectDialog::AddObjectDialog(const int w, const int h) : 
ScrollList("menu/background_box_dark.png", "small", w, h), selected_z(0) {
	_base = "data";
	_fname = "editor.xml";
	
	std::string src = Finder->find(_base, _fname, false);
	if (!src.empty()) {
		scoped_ptr<mrt::BaseFile> ptr(Finder->get_file(src, "rt"));
		parseFile(*ptr);
	}

	Classes classes;
	ResourceManager->getAllClasses(classes);
	for(Classes::iterator i = classes.begin(); i != classes.end(); ++i) 
		_classes.insert(*i);
		
	//control initialization
	std::string classname;
	for(Animations::const_iterator i = _animations.begin(); i != _animations.end(); ++i) {
		if (!ResourceManager->hasClass(i->first))
			continue;
		
		if (i->first != classname) {
			classname = i->first;
			//check for single-items
			bool single_item = false;
			Animations::const_iterator j = i;
			++j;
			if (j == _animations.end() || j->first != classname)
				single_item = true;
			if (!single_item) {
				_user_data.push_back(_animations.end());
				append(i->first + ":");
				_user_data.push_back(i);
				append("  " + i->second);
			} else {
				_user_data.push_back(i);
				append(i->first + ": " + i->second);
			}
		} else {
			_user_data.push_back(i);
			append("  " + i->second);
		}
	}

	_z = new ObjectPropertiesDialog(w);
	int cw, ch;
	getSize(cw, ch);
	add(0, ch, _z);
}


void AddObjectDialog::start(const std::string &name, Attrs &attr) {
	if (name == "editor")
		return;
	else if (name == "class") {
		if (!attr["id"].empty()) {
			_classname = attr["id"];
			_classes.insert(_classname);
		} else {
			LOG_WARN(("class tag with empty id. skipped"));
		}
	} else if (name == "animation") {
		if (_classname.empty()) {
			LOG_ERROR(("animation must be inside class tag"));
			return;
		}
		if (!attr["id"].empty()) {
			_animations.insert(Animations::value_type(_classname, attr["id"]));
		} else {
			LOG_WARN(("animation tag with empty id. skipped"));
		}		
	} else if (name == "variant") {
		if (_classname.empty()) {
			LOG_ERROR(("variant must be inside class tag"));
			return;
		}
		if (!attr["id"].empty()) {
			_variants.insert(Animations::value_type(_classname, attr["id"]));
		} else {
			LOG_WARN(("variant tag with empty id. skipped"));
		}
	}
}

bool AddObjectDialog::onKey(const SDL_keysym sym) {
	switch(sym.sym) {
	case SDLK_LCTRL:
	case SDLK_RETURN: 
		{	
			Animations::const_iterator i = _user_data[ScrollList::get()];
			if (i == _animations.end())
				return true;
			selected_classname = i->first;
			selected_animation = i->second;
			selected_z = _z->getZ();
		}
		invalidate();
		hide();
		return true;
	case SDLK_ESCAPE: 
		hide();
		return true;
	default: 
		return ScrollList::onKey(sym);	
	}
}


void AddObjectDialog::end(const std::string &name) {
	if (name == "editor")
		return;
	else if (name == "class") {
		_classname.clear();
	}
}

void AddObjectDialog::getVariants(std::set<std::string> &variants, const std::string &classname) const {
	variants.clear();
	Variants::const_iterator b = _variants.lower_bound(classname);
	Variants::const_iterator e = _variants.upper_bound(classname);
	for(Variants::const_iterator j = b; j != e; ++j) {
		variants.insert(j->second);
	}
}

AddObjectDialog::~AddObjectDialog() {
	TRY {
		if (Finder->packed(_base))
			return;
		
		std::string fname = _base + "/" + _fname;
		LOG_DEBUG(("saving editor.xml file to %s", fname.c_str()));
		mrt::File file;
		file.open(fname, "wb");
		
		std::string xml;
		xml = 	"<?xml version=\"1.0\"?>\n<editor>\n"
				"\t<!-- add <animation id=\"sample animation\" /> inside <class> tag\n"
				"\t\tto make object visible in \"add new object dialog\" -->\n";
		
		for(Classes::const_iterator i = _classes.begin(); i != _classes.end(); ++i) {
			xml += mrt::formatString("\t<class id=\"%s\">\n", i->c_str()); 
			{
				Animations::const_iterator b = _animations.lower_bound(*i);
				Animations::const_iterator e = _animations.upper_bound(*i);
				for(Animations::const_iterator j = b; j != e; ++j) {
					xml += mrt::formatString("\t\t<animation id=\"%s\" />\n", j->second.c_str());
				}
			}

			{
				Variants::const_iterator b = _variants.lower_bound(*i);
				Variants::const_iterator e = _variants.upper_bound(*i);
				for(Variants::const_iterator j = b; j != e; ++j) {
					xml += mrt::formatString("\t\t<variant id=\"%s\" />\n", j->second.c_str());
				}
			}
				
			xml += "\t</class>\n";
		}
		
		xml += "</editor>\n";
		try { 
			mrt::Directory dir;
			dir.create(_base, true);
		} catch(...) {}
		file.writeAll(xml);
		file.close();
		
	} CATCH("AddObjectDialog dtor", );
}

const bool AddObjectDialog::get(std::string &classname, std::string &animation, int &z) {
	if (selected_classname.empty() || selected_animation.empty())
		return false;

	classname = selected_classname; 
	animation = selected_animation;
	z = selected_z;

	selected_animation.clear();
	selected_classname.clear();
	return true;
}

void AddObjectDialog::tick(const float dt) {
	ScrollList::tick(dt);
	if (changed()) {
		reset();
		_z->reset();
	}
}
