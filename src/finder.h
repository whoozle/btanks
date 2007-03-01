#ifndef BTANKS_FINDER_H__
#define BTANKS_FINDER_H__

#include "mrt/singleton.h"
#include <string>
#include <vector>

class IFinder {
public: 
	typedef std::vector<std::pair<std::string, std::string> > FindResult;

	DECLARE_SINGLETON(IFinder);

	IFinder();	

	const std::string find(const std::string &name) const;
	void findAll(FindResult &result, const std::string &name) const;

	void getPath(std::vector<std::string> &path) const;

private: 
	std::vector<std::string> _path;
};

SINGLETON(Finder, IFinder);

#endif

