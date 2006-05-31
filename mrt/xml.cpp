#include "xml.h"
#include "fmt.h"
#include "file.h"

using namespace mrt;

XMLException::XMLException() {}


const std::string XMLException::getCustomMessage() { return ""; }

XMLException::~XMLException() throw() {}

static void XMLCALL startElement(void *userData, const char *name, const char **attrs) {
	XMLParser * p = (XMLParser *)userData;
	XMLParser::Attrs attrs_map;
	const char ** c =attrs;
	while (*c) {
		const std::string key = *c++; 
		if (*c == NULL)
			throw_ex(("unpaired attribute (%s)", key.c_str()));
		const std::string value = *c++;
		attrs_map[key] = value;
	}
	p->start(name, attrs_map);
}

static void XMLCALL endElement(void *userData, const char *name) {
	XMLParser * p = (XMLParser *)userData;
	p->end(name);
}

static void XMLCALL char_data(void *userData, const XML_Char *s, int len) {
	XMLParser * p = (XMLParser *)userData;
	p->charData(std::string(s, len));
}

void XMLParser::parseFile(const std::string &fname) {
	_parser = XML_ParserCreate("UTF-8");
	if (_parser == NULL)
		throw_ex(("cannot create parser"));
	XML_SetUserData(_parser, this);
	XML_SetElementHandler(_parser, startElement, endElement);
	XML_SetCharacterDataHandler(_parser, char_data);
	
	mrt::File f;
	f.open(fname, "rt");
	bool done;
	do {
		char buf[16384];
		size_t len = f.read(buf, sizeof(buf));
		done = len < sizeof(buf);
		if (XML_Parse(_parser, buf, len, done) == XML_STATUS_ERROR) 
			throw_xml(this);
	} while(!done);
	f.close();
}

const std::string XMLParser::getErrorMessage() const {
	return mrt::formatString("%s at line %d", 
				XML_ErrorString(XML_GetErrorCode(_parser)),
				XML_GetCurrentLineNumber(_parser));
}

	
void XMLParser::charData(const std::string &data) {}
	
XMLParser::~XMLParser() {}
