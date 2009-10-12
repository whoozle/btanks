#ifndef __BTANKS_SERVER_H__
#define __BTANKS_SERVER_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
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

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/


#include "mrt/tcp_socket.h"
#include "mrt/udp_socket.h"

class PlayerState;
class Message;
class Monitor;

class Server {
public:
	Server(); 
	void init();
	void send(const int id, const Message &m);
	void broadcast(const Message &m);
	void tick(const float dt);
	~Server();
	
	const bool active() const;
	void disconnect(const int id);
	
	void restart();
	void disconnect_all();
	
private:
	Server(const Server &);
	const Server& operator=(const Server &);
	Monitor *_monitor;
	
	mrt::TCPSocket _sock;
	mrt::UDPSocket _udp_sock;
};


#endif

