#include "nickname.h"
#include "i18n.h"
#include "mrt/random.h"
#include "mrt/logger.h"

#define RANDOM_ITEM(q) ((q)[mrt::random((q).size())])

const std::string Nickname::generate() {
	std::deque<std::string> prefixes, suffixes, roots;
	
	I18n->enumerateKeys(prefixes, "names/prefixes");
	I18n->enumerateKeys(roots, "names/roots");
	I18n->enumerateKeys(suffixes, "names/suffixes");
	if (prefixes.empty() || roots.empty() || suffixes.empty())
		throw_ex(("nick generation requires proper setup in names/ area of strings.xml"));
	
	int n = mrt::random(3);

	bool has_prefix = mrt::random(100) >= 80;
	bool has_suffix = mrt::random(100) < 80;

	std::string name;
	
	if (has_prefix) {
		name = I18n->get("names/prefixes", RANDOM_ITEM(prefixes));
	} else {
		name = I18n->get("names/roots", RANDOM_ITEM(roots));
	}

	while(n--) 
		name += I18n->get("names/roots", RANDOM_ITEM(roots));

	if (has_suffix) {
		name += I18n->get("names/suffixes", RANDOM_ITEM(suffixes));
	} else {
		name += I18n->get("names/roots", RANDOM_ITEM(roots));
	}

	return name;	
}
