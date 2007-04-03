
/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
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

NotifyingXMLParser::NotifyingXMLParser() : reset_progress(), notify_progress() {}


void NotifyingXMLParser::parseFile(const std::string &fname) {
	int tags;
	getFileStats(tags, fname);
	reset_progress.emit(tags);
	XMLParser::parseFile(fname);
}

void NotifyingXMLParser::parseFiles(const std::vector<std::pair<std::string, std::string> > &files) {
	int progress = 0;
	for(size_t i = 0; i < files.size(); ++i) {
		int tags;
		getFileStats(tags, files[i].second);
		progress += tags;
	}
	
	reset_progress.emit(progress);

	for(size_t i = 0; i < files.size(); ++i) {
		onFile(files[i].first, files[i].second);
		XMLParser::parseFile(files[i].second);
	}
}

void NotifyingXMLParser::start(const std::string &name, Attrs &attr) {}

void NotifyingXMLParser::end(const std::string &name) {
	notify_progress.emit(1);
}
