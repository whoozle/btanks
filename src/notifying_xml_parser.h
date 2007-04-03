#ifndef __BTASKS_NOTIFYING_XML_PARSER_H__
#define __BTASKS_NOTIFYING_XML_PARSER_H__

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

#include "mrt/xml.h"
#include <vector>
#include <sigc++/sigc++.h>

class NotifyingXMLParser : public sigc::trackable, public mrt::XMLParser {
public: 
	NotifyingXMLParser();
	sigc::signal1<void, const int> reset_progress;
	sigc::signal1<void, const int> notify_progress;

protected:	
	virtual void parseFile(const std::string &fname);
	virtual void onFile(const std::string &base, const std::string &file) {}
	virtual void parseFiles(const std::vector<std::pair<std::string, std::string> > &files);

	virtual void start(const std::string &name, Attrs &attr);
	virtual void end(const std::string &name);

};

#endif

