#ifndef __BT_XML_H__
#define __BT_XML_H__

#include <map>
#include <string>

#include "exception.h"
#include "expat.h"

namespace mrt {

DERIVE_EXCEPTION(XMLException);

#define throw_xml(parser) { \
	mrt::XMLException e; \
	e.addMessage(__FILE__, __LINE__); \
	e.addMessage("XML error" + parser->getErrorMessage()); throw e; }

class XMLParser {
public:
	typedef std::map<const std::string, std::string> Attrs;

	void parseFile(const std::string &fname);
	const std::string getErrorMessage() const;
	
	virtual void start(const std::string &name, const Attrs &attr) = 0;
	virtual void end(const std::string &name) = 0;
	virtual void charData(const std::string &data);
	
	virtual ~XMLParser();
private:
	XML_Parser _parser;
};
}

#endif
