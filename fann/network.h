#ifndef __FANNCXX_NETWORK_H__
#define __FANNCXX_NETWORK_H__

#include <fann.h>
#include <string>

namespace fanncxx {

class Network {
public: 
	enum Type {Standard, Sparse, Shortcut };
	Network();
	
	const bool isNull() const { return network == NULL; }
	
	void load(const std::string &file);
	void create(const Type type, const int layers_num, const unsigned int *layers, const float connection_rate = 1 /*used only in Sparse type*/);
	
	void randomizeWeights(const fann_type min, const fann_type max);
	
	fann_type *run(fann_type * input);
	void train(fann_type *input, fann_type 	*desired_output);
	

	void printConnections();
	void printParameters();
	
	const unsigned int getNumInput() const;
	const unsigned int getNumOutput() const;
	const unsigned int getTotalNeurons() const;
	const unsigned int getTotalConnections() const;
	const float getMSE() const;
	
	void destroy();
	~Network();
	
	void save(const std::string &file);
private: 
	struct fann * network;
};
}

#endif
