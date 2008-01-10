
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
#include "finder.h"
#include "config.h"
#include "mrt/directory.h"
#include "mrt/fmt.h"
#include "mrt/file.h"
#include "zzip/mrt_zzip_file.h"
#include "zzip/mrt_zzip_dir.h"
#include <algorithm>
#include "utils.h"
#include "scoped_ptr.h"

IMPLEMENT_SINGLETON(Finder, IFinder);

struct Package {
	std::set<std::string> files;
	zzip::Directory root;
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
	return package->root.open_file(name);
}

const bool IFinder::exists(const std::string &name) const {
	for(Packages::const_iterator i = packages.begin(); i != packages.end(); ++i) {
		const Package * package = i->second;
		if (package->files.find(name) != package->files.end())
			return true;	
	}

	mrt::Directory dir;
	if (dir.exists(name))
		return true;
	
	return true;
}

IFinder::IFinder() {
	std::string path;
#ifdef PREFIX
	Config->get("engine.path", path, RESOURCES_DIR "/private/data:" RESOURCES_DIR "/data");
#else
	Config->get("engine.path", path, "private/data:data");
#endif
	LOG_DEBUG(("engine.path = %s", path.c_str()));
	std::vector<std::string> r;
	mrt::split(r, path, ":");
	mrt::Directory dir;
	for(size_t i = 0; i < r.size(); ++i) {
		LOG_DEBUG(("checking for directory: %s", r[i].c_str()));
		bool found = false;
		if (dir.exists(r[i])) {
			_path.push_back(r[i]);
			found = true;
		} 
		std::string dat = mrt::FSNode::getParentDir(r[i]) + "/resources.dat";
		LOG_DEBUG(("checking for compressed resources in %s", dat.c_str()));
		if (dir.exists(dat)) {
			found = true;
			LOG_DEBUG(("found packed resources, adding %s to the list", dat.c_str()));

			scoped_ptr<Package> package(new Package);
			package->root.open(dat);
			std::string file;
			while(!(file = package->root.read()).empty()) {
				//LOG_DEBUG(("file: %s", file.c_str()));
				package->files.insert(file);
			}
			LOG_DEBUG(("%u files were read from the archive", (unsigned)package->files.size()));
			delete packages[r[i]];
			packages[r[i]] = package.release();
		} 
		
		if (!found)
			LOG_DEBUG(("skipped non-existent path item %s", r[i].c_str()));
	}
	if (_path.empty())
		throw_ex(("none of the directories listed in engine.path('%s') exist", path.c_str()));
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
	for(size_t j = 0; j < files.size(); ++j) {
		//LOG_DEBUG(("looking for the file: %s", files[j].c_str()));
		if (exists(files[j]))
			return files[j];
	}
	if (strict)
		throw_ex(("file '%s' not found", file.c_str()));
	return std::string();
}


const std::string IFinder::find(const std::string &name, const bool strict) const {
	for(size_t i = 0; i < _path.size(); ++i) {
		std::vector<std::string> files;
		applyPatches(files, _path[i] + "/" + name);
		for(size_t j = 0; j < files.size(); ++j) {
			//LOG_DEBUG(("looking for the file: %s", files[j].c_str()));
			if (exists(files[j]))
				return files[j];
		}
	}
	if (strict)
		throw_ex(("file '%s' not found", name.c_str()));
	return std::string();
}

void IFinder::findAll(FindResult &result, const std::string &name) const {
	result.clear();
	
	for(size_t i = 0; i < _path.size(); ++i) {
		std::vector<std::string> files;
		applyPatches(files, _path[i] + "/" + name);
		for(size_t j = 0; j < files.size(); ++j) {
			//LOG_DEBUG(("looking for the file: %s", files[j].c_str()));
			if (exists(files[j])) {
				result.push_back(FindResult::value_type(_path[i], files[j]));
				break;
			}
		}
	}
}

void IFinder::getPath(std::vector<std::string> &path) const {
	path = _path;
}

void IFinder::addPatchSuffix(const std::string &patch) {
	patches.push_back(patch);
}
