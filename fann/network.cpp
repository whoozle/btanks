#include "network.h"
#include "mrt/exception.h"
#include "./exception.h"

using namespace fanncxx;

#define throw_fnet(args) throw_generic_no_default(Exception, args, ((struct fann_error *)network))

Network::Network() : network(NULL) {}

void Network::create(const Type type, const int layers_num, const unsigned int *layers, const float connection_rate) {
	destroy();
	
	switch(type) {
		case Standard:
			network = fann_create_standard_array(layers_num, const_cast<unsigned int *>(layers));
		case Sparse:
			network = fann_create_sparse_array(connection_rate, layers_num, const_cast<unsigned int *>(layers));
		case Shortcut:
			network = fann_create_shortcut_array(layers_num, const_cast<unsigned int *>(layers));
	}
	if (network == NULL)
		throw_fnet(("fann_create_XXX_array(%d, %p, %g) (type %d)", layers_num, (void*)layers, connection_rate, type));
}

void Network::randomize_weights(const fann_type min, const fann_type max) {
	fann_randomize_weights(network, min, max);
}

fann_type *Network::run(fann_type * input) {
	fann_type * r = fann_run(network, input);
	if (r == NULL)
		throw_fnet(("fann_run"));
	return r;
}

void Network::train(fann_type *input, fann_type 	*desired_output) {
	fann_train(network, input, desired_output);
}


void Network::printConnections() {
	fann_print_connections(network);
}

void Network::printParameters() {
	fann_print_parameters(network);
}

const unsigned int Network::getNumInput() {
	return fann_get_num_input(network);
}
const unsigned int Network::getNumOutput() {
	return fann_get_num_output(network);
}
const unsigned int Network::getTotalNeurons() {
	return fann_get_total_neurons(network);
}
const unsigned int Network::getTotalConnections() {
	return fann_get_total_connections(network);
}

void Network::load(const std::string &file) {
	destroy();
	network = fann_create_from_file(file.c_str());
	if (network == NULL)
		throw_ex(("fann_create_from_file('%s')", file.c_str()));
}

void Network::save(const std::string &file) {
	if (fann_save(network, file.c_str()) != 0)
		throw_fnet(("fann_save"));
}

void Network::destroy() {
	if (network == NULL)
		return;
	fann_destroy(network);
	network = NULL;
}

Network::~Network() {
	destroy();
}
