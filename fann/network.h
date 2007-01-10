#ifndef __FANNCXX_NETWORK_H__
#define __FANNCXX_NETWORK_H__

#include <fann.h>

namespace fann {

class Network {
public: 
	enum Type {Standard, Sparse, Shortcut };
	Network(const Type type, const int layers_num, const unsigned int *layers, const float connection_rate = 1 /*used only in Sparse type*/);
	
	void randomize_weights(const fann_type min, const fann_type max);
	fann_type *run(fann_type * input);

	void printConnections();
	void printParameters();
	
	const unsigned int getNumInput();
	const unsigned int getNumOutput();
	const unsigned int getTotalNeurons();
	const unsigned int getTotalConnections();
	
	~Network();
private: 
	struct fann * network;
};
}

#endif
