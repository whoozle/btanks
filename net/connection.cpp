#include "connection.h"
#include "mrt/tcp_socket.h"

Connection::Connection(mrt::TCPSocket *s, const int id) : id(id), sock(s) {}
Connection::~Connection() { delete sock; sock = NULL; }
