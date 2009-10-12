/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/

#include "add_object_dialog.h"
#include "mrt/file.h"
#include "mrt/directory.h"
#include "finder.h"
#include "mrt/exception.h"
#include "resource_manager.h"
#include <algorithm>
#include "object_properties.h"
#include "mrt/scoped_ptr.h"

AddObjectDialog::AddObjectDialog(const int w, const int h) : 
ScrollList("menu/background_box_dark.png", "small", w, h), selected_z(0) {
	_base = Finder->get_base_path();
	_fname = "editor.xml";
	
	std::string src = Finder->find(_base, _fname, false);
	if (!src.empty()) {
		scoped_ptr<mrt::BaseFile> ptr(Finder->get_file(src, "rt"));
		parse_file(*ptr);
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
	get_size(cw, ch);
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
	case SDLK_KP_ENTER:
	case SDLK_RETURN: 
		{	
			Animations::const_iterator i = _user_data[ScrollList::get()];
			if (i == _animations.end())
				return true;
			selected_classname = i->first;
			selected_animation = i->second;
			selected_z = _z->get_z();
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

void AddObjectDialog::get_variants(std::set<std::string> &variants, const std::string &classname) const {
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
			xml += mrt::format_string("\t<class id=\"%s\">\n", i->c_str()); 
			{
				Animations::const_iterator b = _animations.lower_bound(*i);
				Animations::const_iterator e = _animations.upper_bound(*i);
				for(Animations::const_iterator j = b; j != e; ++j) {
					xml += mrt::format_string("\t\t<animation id=\"%s\" />\n", j->second.c_str());
				}
			}

			{
				Variants::const_iterator b = _variants.lower_bound(*i);
				Variants::const_iterator e = _variants.upper_bound(*i);
				for(Variants::const_iterator j = b; j != e; ++j) {
					xml += mrt::format_string("\t\t<variant id=\"%s\" />\n", j->second.c_str());
				}
			}
				
			xml += "\t</class>\n";
		}
		
		xml += "</editor>\n";
		try { 
			mrt::Directory dir;
			dir.create(_base, true);
		} catch(...) {}
		file.write_all(xml);
		file.close();
		
	} CATCH("AddObjectDialog dtor", {});
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
