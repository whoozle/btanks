#ifndef __BTANKS_SYS_SOCKET_H__
#define __BTANKS_SYS_SOCKET_H__

namespace mrt {
	class Socket {
	public:
		Socket();
		static void init();
		static const int create(const int af, int type, int protocol);
		
		const int getFD() const { return _sock; }
		void close(); 
		
		~Socket();
	protected: 
		int _sock;
	};
}

#endif
