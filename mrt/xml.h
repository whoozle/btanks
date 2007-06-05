#ifndef __BT_XML_H__
#define __BT_XML_H__

/* M-Runtime for c++
 * Copyright (C) 2005-2007 Vladimir Menshakov
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

#include <map>
#include <string>
#include <expat.h>

#include "exception.h"
#include "export_mrt.h"

namespace mrt {

DERIVE_EXCEPTION(MRTAPI, XMLException);

class MRTAPI XMLParser {
public:
	typedef std::map<const std::string, std::string> Attrs;

	static void getFileStats(int &tags, const std::string &fname);
	virtual void parseFile(const std::string &fname);
	
	virtual void start(const std::string &name, Attrs &attr) = 0;
	virtual void end(const std::string &name) = 0;
	virtual void charData(const std::string &data);
	
	void clear();
	XMLParser();
	virtual ~XMLParser();

	XMLParser(const XMLParser &);
	const XMLParser& operator=(const XMLParser &);

private:
	const std::string getErrorMessage() const;
	XML_Parser _parser;
};
}

#endif
