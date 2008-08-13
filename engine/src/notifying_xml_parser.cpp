
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
#include "notifying_xml_parser.h"
#include "mrt/scoped_ptr.h"
#include "finder.h"
#include "mrt/base_file.h"

NotifyingXMLParser::NotifyingXMLParser() : reset_progress(), notify_progress(), status(NULL) {}


void NotifyingXMLParser::parse_file(const std::string &file) {
	scoped_ptr<mrt::BaseFile> f(Finder->get_file(file, "rt"));
	parse_file(*f);
	f->close();
}

void NotifyingXMLParser::parse_file(const mrt::BaseFile &file) {
	int tags;
	get_file_stats(tags, file);
	reset_progress.emit(tags);
	mrt::XMLParser::parse_file(file);
}

void NotifyingXMLParser::parse_files(const std::vector<std::pair<std::string, std::string> > &files) {
	int progress = 0;
	for(size_t i = 0; i < files.size(); ++i) {
		int tags;
		scoped_ptr<mrt::BaseFile> f(Finder->get_file(files[i].second, "rt"));
		get_file_stats(tags, *f);
		progress += tags;
	}
	
	reset_progress.emit(progress);

	for(size_t i = 0; i < files.size(); ++i) {
		scoped_ptr<mrt::BaseFile> f(Finder->get_file(files[i].second, "rt"));
		onFile(files[i].first, files[i].second);
		mrt::XMLParser::parse_file(*f);
	}
}

void NotifyingXMLParser::start(const std::string &name, Attrs &attr) {}

void NotifyingXMLParser::end(const std::string &name) {
	notify_progress.emit(1, status);
}
