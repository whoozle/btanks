#include "client.h"
#include "mrt/logger.h"

Client::Client(): _running(false) {}

void Client::init(const std::string &host, const unsigned port) {
	LOG_DEBUG(("connecting to %s:%u [stub]", host.c_str(), port));
	_sock.connect(host, port);
	_running = true;
}
