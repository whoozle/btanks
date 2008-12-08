#ifndef __BT_XML_H__
#define __BT_XML_H__

/* M-runtime for c++
 * Copyright (C) 2005-2008 Vladimir Menshakov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/


#include <map>
#include <string>
#include <expat.h>

#include "exception.h"
#include "export_mrt.h"

namespace mrt {
class BaseFile;

DERIVE_EXCEPTION(MRTAPI, XMLException);

class MRTAPI XMLParser {
public:
	struct Attrs : public std::map<const std::string, std::string> {
		inline bool get(const std::string &name, bool defv) const {
			const_iterator i = find(name);
			return i != end()? (i->second == "true" || i->second == "yes"): defv;
		}
		inline int get(const std::string &name, int defv) const {
			const_iterator i = find(name);
			return i != end()? (int)atoi(i->second.c_str()): defv;
		}
		inline float get(const std::string &name, float defv) const {
			const_iterator i = find(name);
			return i != end()? (float)atof(i->second.c_str()): defv;
		}
		inline const std::string get(const std::string &name, const char * defv) const {
			const_iterator i = find(name);
			return i != end()? i->second: std::string(defv);
		}
		inline const std::string get(const std::string &name, const std::string &defv) const {
			const_iterator i = find(name);
			return i != end()? i->second: defv;
		}
	};

	static void get_file_stats(int &tags, const std::string &fname);
	static void get_file_stats(int &tags, const mrt::BaseFile &file);
	virtual void parse_file(const std::string &fname);
	virtual void parse_file(const mrt::BaseFile &file);
	
	virtual void start(const std::string &name, Attrs &attr) = 0;
	virtual void end(const std::string &name) = 0;
	virtual void cdata(const std::string &data);
	
	void clear();
	XMLParser();
	virtual ~XMLParser();

	XMLParser(const XMLParser &);
	const XMLParser& operator=(const XMLParser &);
	
	static const std::string escape(const std::string &str);

private:
	const std::string getErrorMessage() const;
	XML_Parser _parser;
};
}

#endif
