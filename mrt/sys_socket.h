#ifndef __BTANKS_SYS_SOCKET_H__
#define __BTANKS_SYS_SOCKET_H__

/* M-runtime for c++
 * Copyright (C) 2005-2008 Vladimir Menshakov
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "export_mrt.h"
#include <string>

#ifdef _WINDOWS
#	ifndef mrt_uint32_t
#		define mrt_uint32_t unsigned __int32
#	endif
#	ifndef mrt_uint16_t
#		define mrt_uint16_t unsigned __int16
#	endif
#else
	typedef uint32_t mrt_uint32_t;
	typedef uint16_t mrt_uint16_t;
#endif

namespace mrt {
	class MRTAPI Socket {
	public:
		struct MRTAPI addr {
			mrt_uint32_t ip;
			mrt_uint16_t port;

			addr() : ip(0), port(0) {}
			addr(unsigned ip, unsigned port) : ip(ip), port(port) {}
			
			const bool empty() const { return ip == 0; }
			const bool operator==(const addr &other) const {
				return ip == other.ip && port == other.port;
			}
			void getAddr(const std::string &name);
			const std::string getAddr() const;
			const std::string getName() const; //gethostbyaddr
			void parse(const std::string &ip); 
		};

		Socket();
		static void init();
		void create(const int af, int type, int protocol);
		
		void close(); 
		virtual ~Socket();
	protected: 
		int _sock;

		friend class SocketSet;
	};
}

#endif
