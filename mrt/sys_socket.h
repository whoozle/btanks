#ifndef __BTANKS_SYS_SOCKET_H__
#define __BTANKS_SYS_SOCKET_H__

namespace mrt {
	class Socket {
	public:
		Socket();
		static void init();
		void create(const int af, int type, int protocol);
		
		void close(); 
		~Socket();
	protected: 
		int _sock;
	};
}

#endif
