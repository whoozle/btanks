/* M-runtime for c++
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
#include "xml.h"
#include "fmt.h"
#include "file.h"

using namespace mrt;

#define throw_xml(parser) { \
	mrt::XMLException e; \
	e.addMessage(__FILE__, __LINE__); \
	e.addMessage("XML error" + parser->getErrorMessage()); throw e; }


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

static void XMLCALL startElementStats(void *userData, const char *name, const char **attrs) {}

static void XMLCALL endElementStats(void *userData, const char *name) {
	int * pr = (int*) userData;
	++ (*pr);
}

static void XMLCALL char_data(void *userData, const XML_Char *s, int len) {
	XMLParser * p = (XMLParser *)userData;
	p->charData(std::string(s, len));
}

void XMLParser::getFileStats(int &tags, const std::string &fname) {
	XML_Parser parser = NULL;

	TRY {
		parser = XML_ParserCreate("UTF-8");
		if (parser == NULL)
			throw_ex(("cannot create parser"));

		tags = 0;
		XML_SetUserData(parser, &tags);
		XML_SetElementHandler(parser, startElementStats, endElementStats);

		mrt::File f;
		f.open(fname, "rt");
		bool done;
		do {
			char buf[16384];
			size_t len = f.read(buf, sizeof(buf));
			done = len < sizeof(buf);
			if (XML_Parse(parser, buf, len, done) == XML_STATUS_ERROR) {
				mrt::XMLException e;
				std::string error = mrt::formatString("%s at line %d", XML_ErrorString(XML_GetErrorCode(parser)), XML_GetCurrentLineNumber(parser));
				e.addMessage("XML error: " + error); throw e; 
			}
		} while(!done);
		XML_ParserFree(parser);
		parser = NULL;
		f.close();
	} CATCH("getFileStats", {
		if (parser) {
			XML_ParserFree(parser);
		}
	})
}


void XMLParser::parseFile(const std::string &fname) {
	clear();
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
	clear();
}

const std::string XMLParser::getErrorMessage() const {
	return mrt::formatString("%s at line %d", 
				XML_ErrorString(XML_GetErrorCode(_parser)),
				XML_GetCurrentLineNumber(_parser));
}

	
void XMLParser::charData(const std::string &data) {}
	
XMLParser::XMLParser() : _parser(0) {}
	

XMLParser::~XMLParser() {
	clear();
}

void XMLParser::clear() {
	if (_parser) {
		XML_ParserFree(_parser);
		_parser = NULL;
	}
}

XMLParser::XMLParser(const XMLParser &) : _parser(NULL) {}
const XMLParser& XMLParser::operator=(const XMLParser &) {
	clear();
	return *this;
}
