
/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
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
#include "finder.h"
#include "config.h"
#include "mrt/directory.h"
#include "mrt/fmt.h"
#include "mrt/file.h"
#include "mrt/zip_file.h"
#include "mrt/zip_dir.h"
#include <algorithm>
#include "utils.h"
#include "mrt/scoped_ptr.h"

IMPLEMENT_SINGLETON(Finder, IFinder);

struct Package {
	//std::set<std::string> files;
	mrt::ZipDirectory *root;
};

mrt::BaseFile *IFinder::get_file(const std::string &file, const std::string &mode) const {
	std::string::size_type p = file.find(':');
	if (p == std::string::npos) {
		mrt::File *f = new mrt::File();
		TRY {
			f->open(file, mode);
		} CATCH("fs open", { delete f; throw; } )
		return f;
	} 
	
	std::string pack = file.substr(0, p);
	Packages::const_iterator i = packages.find(pack);
	if (i == packages.end())
		throw_ex(("invalid package id '%s'", pack.c_str()));

	const Package * package = i->second;
	std::string name = mrt::FSNode::normalize(file.substr(p + 1));
	return package->root->open_file(name);
}

const bool IFinder::exists(const std::string &base, const std::string &name) const {
	Packages::const_iterator i = packages.find(base);
	if (i != packages.end() && i->second->root->exists(name))
		return true;

	mrt::Directory dir;
	return dir.exists(mrt::FSNode::normalize(base + "/" + name));
}

const bool IFinder::exists(const std::string &name) const {
	for(Packages::const_iterator i = packages.begin(); i != packages.end(); ++i) {
		const Package * package = i->second;
		if (package->root->exists(name))
			return true;	
	}

	mrt::Directory dir;
	for(size_t i = 0; i < _path.size(); ++i) {
		if (dir.exists(_path[i] + "/" + name))
			return true;
	}
	
	return true;
}

//#define PLUGINS_DIR "/usr/lib/games/btanks" //just for fun :)

void IFinder::scan(std::vector<std::string> &path) {
	mrt::Directory dir;

#ifdef RESOURCES_DIR
	dir.open(RESOURCES_DIR);
#else
	dir.open(".");
#endif

	std::string base_dir;
	while(!(base_dir = dir.read()).empty()) {
		if (base_dir[0] == '.' || !mrt::FSNode::is_dir(base_dir))
			continue;
		
		TRY {
			std::string dname = base_dir + "/data";
			std::string rname = base_dir + "/resources.dat";
			if (mrt::FSNode::is_dir(dname) || dir.exists(rname)) {
					//LOG_DEBUG(("data_dir = %s", dname.c_str()));
					path.push_back(dname.c_str());
#ifdef PLUGINS_DIR
					path.push_back(std::string(PLUGINS_DIR "/") + dname); //plugins loaded from path ../bt_objects.
#endif
			}
		} CATCH("scan", )
	}
#ifdef RESOURCES_DIR
	std::string dname = RESOURCES_DIR "/data";
	std::string rname = RESOURCES_DIR "/resources.dat";
#else
	std::string dname = "data";
	std::string rname = "resources.dat";
#endif

	if (mrt::FSNode::is_dir(dname) || dir.exists(rname)) {
		path.push_back(dname);
		_base_path = dname;
#ifdef PLUGINS_DIR
		path.push_back(PLUGINS_DIR "/data"); //plugins loaded from path ../bt_objects.
#endif
	}
	dir.close();
}

IFinder::IFinder() {
	mrt::Directory dir;

	std::string path;
	Config->get("engine.mods", path, std::string());
	
	LOG_DEBUG(("engine.mods = %s", path.c_str()));

	std::vector<std::string> r;
	mrt::split(r, path, ":");

	scan(r);
	LOG_DEBUG(("base pack found at %s", _base_path.c_str()));
	
	for(size_t i = 0; i < r.size(); ++i) {
		const std::string &p = r[i];
		LOG_DEBUG(("checking for directory: %s", p.c_str()));
		bool found = false;
		if (dir.exists(p)) {
			_path.push_back(p);
			found = true;
		} 
		std::string dat = mrt::FSNode::get_parent_dir(p) + "/resources.dat";
		LOG_DEBUG(("checking for compressed resources in %s", dat.c_str()));
		if (dir.exists(dat)) {
			TRY {
				LOG_DEBUG(("found packed resources, adding %s to the list", dat.c_str()));

				scoped_ptr<Package> package(new Package);
				package->root = new mrt::ZipDirectory(dat);
				/*
				package->root->open(dat);
				std::string file;
				while(!(file = package->root.read()).empty()) {
					//LOG_DEBUG(("file: %s", file.c_str()));
					package->files.insert(file);
				}
				LOG_DEBUG(("%u files were read from the archive", (unsigned)package->files.size()));
				*/
				delete packages[p];
				packages[p] = package.release();
				if (!found)
					_path.push_back(p);
				found = true;
			} CATCH("loading packed resources", );
		} 
		
		if (!found)
			LOG_DEBUG(("skipped non-existent path item %s", p.c_str()));
	}
	if (_path.empty())
		throw_ex(("none of the directories listed in engine.mods('%s') exist", path.c_str()));
}

IFinder::~IFinder() {
	std::for_each(packages.begin(), packages.end(), delete_ptr2<Packages::value_type>());
}

void IFinder::applyPatches(std::vector<std::string>& files, const std::string &file) const {
	files.clear();	
	size_t pos = file.rfind('.');
	size_t spos = file.rfind('/');
	if (spos != std::string::npos && pos != std::string::npos && pos < spos)
		pos = std::string::npos;
	
	for(size_t i = 0; i < patches.size(); ++i) {
		if (pos == std::string::npos) {
			files.push_back(file + patches[i]);
		} else {
			std::string f = file;
			f.insert(pos, patches[i]);
			files.push_back(f);
		}
	}
	files.push_back(file);
}

const std::string IFinder::fix(const std::string &file, const bool strict) const {
	std::vector<std::string> files;
	
	applyPatches(files, file);
	mrt::Directory dir;
	for(size_t j = 0; j < files.size(); ++j) {
		//LOG_DEBUG(("looking for the file: %s", files[j].c_str()));
		if (dir.exists(files[j]))
			return files[j];
	}
	if (strict)
		throw_ex(("file '%s' not found", file.c_str()));
	return std::string();
}


const std::string IFinder::find(const std::string &name_, const bool strict) const {
	for(size_t i = 0; i < _path.size(); ++i) {
		const std::string r = find(_path[i], name_, false);
		if (!r.empty())
			return r;
	}
	if (strict)
		throw_ex(("file '%s' not found", name_.c_str()));
	return std::string();
}

const std::string IFinder::find(const std::string &base, const std::string &name_, const bool strict) const {
	mrt::Directory dir;

	
	std::vector<std::string> files;
	applyPatches(files, name_);

	std::string prefix =  base + "/";
	Packages::const_iterator p_i = packages.find(base);
	for(size_t j = 0; j < files.size(); ++j) {
		const std::string name = mrt::FSNode::normalize(prefix + files[j]);
		//LOG_DEBUG(("looking for the file: %s:%s -> %s", base.c_str(), files[j].c_str(), name.c_str()));
		if (dir.exists(name))
			return name;
		if (p_i != packages.end()) {
			//LOG_DEBUG(("checking for %s in archive", files[j].c_str()));
			std::string n = mrt::FSNode::normalize(files[j]);
			if (p_i->second->root->exists(n))
				return base + ":" + n;
		}
	}
	if (strict)
		throw_ex(("file '%s' not found", name_.c_str()));
	return std::string();
}


void IFinder::findAll(FindResult &result, const std::string &name_) const {
	result.clear();
	for(size_t i = 0; i < _path.size(); ++i) {
		const std::string r = find(_path[i], name_, false);
		if (!r.empty()) {
			result.push_back(FindResult::value_type(_path[i], r));
		}
	}
}

void IFinder::load(mrt::Chunk &data, const std::string &fname, const bool do_find) const {
	std::string name = do_find? find(fname): fname;
	scoped_ptr<mrt::BaseFile> file(get_file(name, "rb"));
	file->read_all(data);
	file->close();
}

void IFinder::getPath(std::vector<std::string> &path) const {
	path = _path;
}

void IFinder::addPatchSuffix(const std::string &patch) {
	patches.push_back(patch);
}

void IFinder::enumerate(std::vector<std::string>&files, const std::string &base, const std::string &root) const {
	files.clear();
	TRY { 
		mrt::Directory dir;
		if (dir.exists(base + "/" + root)) {
			dir.open(base + "/" + root);
			std::string file;
			while(!(file = dir.read()).empty()) {
				files.push_back(file);
			}
			dir.close();
			return;
		}
	} CATCH("scanning directory", );
	
	Packages::const_iterator p_i = packages.find(base);
	if (p_i == packages.end())
		return;

	p_i->second->root->enumerate(files, root);
}

const bool IFinder::packed(const std::string &base) const {
	Packages::const_iterator p_i = packages.find(base);
	if (p_i == packages.end())	
		return false;
	return p_i->second->root != NULL;
}
