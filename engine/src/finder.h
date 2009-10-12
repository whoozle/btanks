#ifndef BTANKS_FINDER_H__
#define BTANKS_FINDER_H__

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

#include "export_btanks.h"
#include "mrt/singleton.h"
#include <string>
#include <vector>
#include <map>

namespace mrt {
	class BaseFile;
	class Chunk;
}

struct Package;

class BTANKSAPI IFinder {
public: 
	typedef std::vector<std::pair<std::string, std::string> > FindResult;

	DECLARE_SINGLETON(IFinder);

	IFinder();	
	~IFinder();	

	const std::string find(const std::string &name, const bool strict = true) const;
	const std::string find(const std::string &base, const std::string &name, const bool strict = true) const;
	void findAll(FindResult &result, const std::string &name) const;

	void getPath(std::vector<std::string> &path) const;
	void addPatchSuffix(const std::string &patch);
	
	const std::string fix(const std::string &file, const bool strict = true) const;
	const bool exists(const std::string &name) const;
	const bool exists(const std::string &base, const std::string &name) const;
	
	mrt::BaseFile *get_file(const std::string &file, const std::string &mode) const;
	void load(mrt::Chunk &data, const std::string &fname, const bool do_find = true) const;
	void enumerate(std::vector<std::string>&files, const std::string &base, const std::string &root) const;
	
	const bool packed(const std::string &base) const;
	
	const std::string get_base_path() const { return _base_path; }

private: 
	void scan(std::vector<std::string> &path);
	void applyPatches(std::vector<std::string>& files, const std::string &fname) const;

	std::vector<std::string> _path;
	std::vector<std::string> patches;
	
	typedef std::map<const std::string, Package*> Packages;
	Packages packages;
	
	std::string _base_path;
};

PUBLIC_SINGLETON(BTANKSAPI, Finder, IFinder);

#endif

