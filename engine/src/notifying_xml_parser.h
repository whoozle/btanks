#ifndef __BTASKS_NOTIFYING_XML_PARSER_H__
#define __BTASKS_NOTIFYING_XML_PARSER_H__

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

#include "mrt/xml.h"
#include <vector>
#include "sl08/sl08.h"

class NotifyingXMLParser : public mrt::XMLParser {
public: 
	NotifyingXMLParser();
	sl08::signal1<void, const int> reset_progress;
	sl08::signal2<void, const int, const char *> notify_progress;

protected:	
	virtual void parse_file(const std::string &file);
	virtual void parse_file(const mrt::BaseFile &file);
	virtual void onFile(const std::string &base, const std::string &file) {}
	virtual void parse_files(const std::vector<std::pair<std::string, std::string> > &files);

	virtual void start(const std::string &name, Attrs &attr);
	virtual void end(const std::string &name);
	
	const char *status;
};

#endif

