#ifndef __BT_XML_H__
#define __BT_XML_H__

#include <map>
#include <string>

#include "exception.h"
#include "expat.h"

namespace mrt {

DERIVE_EXCEPTION(XMLException);

class XMLParser {
public:
	typedef std::map<const std::string, std::string> Attrs;

	void parseFile(const std::string &fname);
	
	virtual void start(const std::string &name, Attrs &attr) = 0;
	virtual void end(const std::string &name) = 0;
	virtual void charData(const std::string &data);
	
	virtual ~XMLParser();
private:
	const std::string getErrorMessage() const;
	XML_Parser _parser;
};
}

#endif
