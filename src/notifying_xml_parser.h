#ifndef __BTASKS_NOTIFYING_XML_PARSER_H__
#define __BTASKS_NOTIFYING_XML_PARSER_H__

#include "mrt/xml.h"
#include <vector>
#include <sigc++/sigc++.h>

class NotifyingXMLParser : public mrt::XMLParser {
public: 
	sigc::signal1<void, const int> reset_progress;
	sigc::signal1<void, const int> notify_progress;
	
	virtual void parseFile(const std::string &fname);
	virtual void onFile(const std::string &base, const std::string &file) {}
	virtual void parseFiles(const std::vector<std::pair<std::string, std::string> > &files);

	virtual void start(const std::string &name, Attrs &attr);
	virtual void end(const std::string &name);

};

#endif

