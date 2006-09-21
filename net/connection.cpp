#include "connection.h"
#include "sdlx/tcp_socket.h"

Connection::Connection(sdlx::TCPSocket *s, const int id) : id(id), sock(s) {}
Connection::~Connection() { delete sock; sock = NULL; }
