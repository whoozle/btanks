#ifndef BTANKS_VARIANTS_H__
#define BTANKS_VARIANTS_H__

#include <set>
#include <string>
#include "export_btanks.h"
#include "mrt/serializable.h"

class BTANKSAPI Variants : public mrt::Serializable {
public: 
	Variants();
	const std::string parse(const std::string &name);
	void update(const Variants &other, const bool remove_old);
	const std::string dump() const;

	const bool has(const std::string &name) const;
	const bool empty() const { return vars.empty(); }
	const bool same(const Variants &variant) const;

	void add(const std::string &name);
	void remove(const std::string &name);

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);

private:
	std::set<std::string> vars;
};

#endif
