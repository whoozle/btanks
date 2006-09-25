#include "connection.h"
#include "mrt/tcp_socket.h"

Connection::Connection(mrt::TCPSocket *s) : sock(s) {}
Connection::~Connection() { delete sock; sock = NULL; }
