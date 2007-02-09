#include "notifying_xml_parser.h"

NotifyingXMLParser::NotifyingXMLParser() : reset_progress(), notify_progress() {}


void NotifyingXMLParser::parseFile(const std::string &fname) {
	int tags;
	getFileStats(tags, fname);
	reset_progress.emit(tags);
	XMLParser::parseFile(fname);
}

void NotifyingXMLParser::parseFiles(const std::vector<std::pair<std::string, std::string> > &files) {
	int progress = 0;
	for(size_t i = 0; i < files.size(); ++i) {
		int tags;
		getFileStats(tags, files[i].second);
		progress += tags;
	}
	
	reset_progress.emit(progress);

	for(size_t i = 0; i < files.size(); ++i) {
		onFile(files[i].first, files[i].second);
		XMLParser::parseFile(files[i].second);
	}
}

void NotifyingXMLParser::start(const std::string &name, Attrs &attr) {}

void NotifyingXMLParser::end(const std::string &name) {
	notify_progress.emit(1);
}
