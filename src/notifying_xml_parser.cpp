#include "notifying_xml_parser.h"

void NotifyingXMLParser::parseFile(const std::string &fname) {
	int tags;
	getFileStats(tags, fname);
	reset_progress.emit(tags);
	XMLParser::parseFile(fname);
}

void NotifyingXMLParser::start(const std::string &name, Attrs &attr) {}

void NotifyingXMLParser::end(const std::string &name) {
	notify_progress.emit(1);
}
