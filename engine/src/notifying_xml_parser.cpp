
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
#include "notifying_xml_parser.h"
#include "mrt/scoped_ptr.h"
#include "finder.h"
#include "mrt/base_file.h"

NotifyingXMLParser::NotifyingXMLParser() : reset_progress(), notify_progress() {}


void NotifyingXMLParser::parseFile(const std::string &file) {
	scoped_ptr<mrt::BaseFile> f(Finder->get_file(file, "rt"));
	parseFile(*f);
	f->close();
}

void NotifyingXMLParser::parseFile(const mrt::BaseFile &file) {
	int tags;
	getFileStats(tags, file);
	reset_progress.emit(tags);
	mrt::XMLParser::parseFile(file);
}

void NotifyingXMLParser::parseFiles(const std::vector<std::pair<std::string, std::string> > &files) {
	int progress = 0;
	for(size_t i = 0; i < files.size(); ++i) {
		int tags;
		scoped_ptr<mrt::BaseFile> f(Finder->get_file(files[i].second, "rt"));
		getFileStats(tags, *f);
		progress += tags;
	}
	
	reset_progress.emit(progress);

	for(size_t i = 0; i < files.size(); ++i) {
		scoped_ptr<mrt::BaseFile> f(Finder->get_file(files[i].second, "rt"));
		onFile(files[i].first, files[i].second);
		mrt::XMLParser::parseFile(*f);
	}
}

void NotifyingXMLParser::start(const std::string &name, Attrs &attr) {}

void NotifyingXMLParser::end(const std::string &name) {
	notify_progress.emit(1);
}
