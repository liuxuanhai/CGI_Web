#include <errno.h>
#include <new>
#include <string.h>
#include "net/ioexception.h"
#include "net/net.h"
#include "net/pollarraywrapper.h"
#include "net/selector.h"
#include "net/selectionkey.h"
#include "net/socketchannel.h"
#include "net/serversocketchannel.h"

namespace lce
{
	namespace net
	{
		// Channel state, increases monotonically
		int ServerSocketChannel::ST_UNINITIALIZED = -1;
		int ServerSocketChannel::ST_INUSE = 0;
		int ServerSocketChannel::ST_KILLED = 1;


		ServerSocketChannel::ServerSocketChannel()
		{
		    SelectableChannel::open();
	        
		    _state = ST_UNINITIALIZED;

		    _fd = Net::serverSocket(true);
		    _state = ST_INUSE;
		}

		ServerSocketChannel::ServerSocketChannel(int fd)
		{
		    SelectableChannel::open();
	        
		    _state = ST_UNINITIALIZED;

		    _fd = fd;
		    _state = ST_INUSE;
		    _local_address = Net::local_address(_fd);
		}


		
		int ServerSocketChannel::get_fd()
		{
		    return _fd;
		}


		// socket options
		// SO_KEEPALIVE
		bool  ServerSocketChannel::keep_alive()
		{
		    return Net::get_bool_option(_fd, SOL_SOCKET,  SO_KEEPALIVE);
		}

		
		void  ServerSocketChannel::keep_alive(bool b)
		{
		    Net::set_bool_option(_fd, SOL_SOCKET,  SO_KEEPALIVE, b);
		}

		// SO_LINGER
		
		int  ServerSocketChannel::linger()
		{
		    return lce::net::Net::get_int_option(_fd, SOL_SOCKET,  SO_LINGER);
		}

		
		void  ServerSocketChannel::linger(int n)
		{
		    Net::set_int_option(_fd, SOL_SOCKET,  SO_LINGER, n);
		}

		// SO_OOBINLINE
		
		bool  ServerSocketChannel::out_of_band_inline()
		{
		    return Net::get_bool_option(_fd, SOL_SOCKET,  SO_OOBINLINE);
		}

		
		void  ServerSocketChannel::out_of_band_inline(bool b)
		{
		    Net::set_bool_option(_fd, SOL_SOCKET,  SO_OOBINLINE, b);
		}

		// SO_RCVBUF
		
		int  ServerSocketChannel::recv_buf_size()
		{
		    return Net::get_int_option(_fd, SOL_SOCKET,  SO_RCVBUF);
		}

		
		void  ServerSocketChannel::recv_buf_size(int n)
		{
		    Net::set_int_option(_fd, SOL_SOCKET,  SO_RCVBUF, n);
		}

		// SO_SNDBUF
		
		int  ServerSocketChannel::send_buf_size()
		{
		    return Net::get_int_option(_fd, SOL_SOCKET,  SO_SNDBUF);
		}

		
		void  ServerSocketChannel::send_buf_size(int n)
		{
		    Net::set_int_option(_fd, SOL_SOCKET,  SO_SNDBUF, n);
		}

		// SO_REUSEADDR
		
		bool  ServerSocketChannel::reuse_addr()
		{
		    return Net::get_bool_option(_fd, SOL_SOCKET,  SO_REUSEADDR);
		}

		
		void  ServerSocketChannel::reuse_addr(bool b)
		{
		    Net::set_bool_option(_fd, SOL_SOCKET,  SO_REUSEADDR, b);
		}

		//  SO_SNDTIMEO
		
		int ServerSocketChannel::send_timeout()
		{
			struct timeval tv;

		       socklen_t arglen =  sizeof(timeval);
			::getsockopt(_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, &arglen);

		       return tv.tv_sec;
		}

		
		void  ServerSocketChannel::send_timeout(int seconds)
		{
			struct timeval tv;

			tv.tv_sec = seconds;
			tv.tv_usec = 0;
			::setsockopt(_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
		}

		// SO_REUSEADDR
		
		bool  ServerSocketChannel::tcp_nodelay()
		{
		    return Net::get_bool_option(_fd, IPPROTO_TCP,  TCP_NODELAY);
		}

		
		void  ServerSocketChannel::tcp_nodelay(bool b)
		{
		    Net::set_bool_option(_fd, IPPROTO_TCP, TCP_NODELAY, b);
		}		

		int ServerSocketChannel::valid_ops()
		{
		    return SelectionKey::OP_ACCEPT;
		}

		bool ServerSocketChannel::is_bound()
		{
		    return _local_address.is_valid();
		}

		InetSocketAddress ServerSocketChannel::local_address()
		{
		    return _local_address;
		}

		int ServerSocketChannel::bind(InetSocketAddress& local, int backlog)
		{
		    int rv;
		    if (!is_open())
		        return -1;
		    if (is_bound())
		        return -1;

		    rv = Net::bind(_fd, local.get_address(), local.get_port());
		    if (rv < 0)
		    {
		        return rv;
		    }

		    rv = listen(_fd, backlog < 1 ? 50 : backlog);
		    if (rv < 0)
		    {
		        return rv;
		    }

		    _local_address = Net::local_address(_fd);

		    return rv;
		}

		int ServerSocketChannel::bind(int port, int backlog)
		{
		    InetSocketAddress sa(port);
		    return bind(sa, backlog);
			
		}

		int  ServerSocketChannel::bind(char* ip, int port, int backlog)
		{
		    InetSocketAddress sa(ip, port);
		    return bind(sa, backlog);
		}

		SocketChannel* ServerSocketChannel::accept()
		{
		    if (!is_open())
		    {
		        return NULL;
		    }
		    if (!is_bound())
		    {
		        return NULL;
		    }

		    int n = 0;
		    int newfd;
		    InetSocketAddress sa;

		    for (; ;)
		    {
		        n = Net::accept(this->_fd, newfd, sa);
		        if (((n == -1) && (errno == EINTR)) && is_open())
		        {
		            continue;
		        }
		        break;
		    }

		    if (n == -1)
		    {
		        return NULL;
		    }

		    Net::configure_blocking(newfd, true);

		    SocketChannel* sc = new SocketChannel();
			sc->attach(newfd, sa);
		    return sc;

		}

		int ServerSocketChannel::accept(SocketChannel& sc)
		{
		    if (!is_open())
		        return -1;
		    if (!is_bound())
		        return -1;

		    int n = 0;
		    int newfd;
		    InetSocketAddress sa;

		    if (!is_open())
		        return -1;

		    for (; ;)
		    {
		        n = Net::accept(this->_fd, newfd, sa);
		        if (((n == -1) && (errno == EINTR)) && is_open())
		            continue;
		        break;
		    }

		    if (n == -1)
		        return -1;

		    Net::configure_blocking(newfd, true);///????

		    SocketChannel* sc_p = &sc;
			sc_p->~SocketChannel();
		    new(sc_p)SocketChannel();
			sc_p->attach( newfd, sa );
		    return 0;
		}

		void ServerSocketChannel::kill()
		{
		    if (_state == ST_KILLED)
		        return;
		    if (_state == ST_UNINITIALIZED)
		    {
		        _state = ST_KILLED;
		        return;
		    }
		    Net::close(_fd);
		    _state = ST_KILLED;
		}

		int ServerSocketChannel::listen(int fd, int backlog)
		{
		    return Net::listen(fd, backlog);
		}


		bool ServerSocketChannel::translate_ready_ops(int ops, int initialOps, SelectionKey& sk)
		{
		    int intOps = sk.nio_interest_ops(); // Do this just once, it synchronizes
		    int oldOps = sk.nio_ready_ops();
		    int newOps = initialOps;

		    if ((ops & Selector::OP_NVAL) != 0)
		    {
		        // This should only happen if this channel is pre-closed while a
		        // selection operation is in progress
		        // ## Throw an error if this channel has not been pre-closed
		        return false;
		    }

		    if ((ops & (Selector::OP_ERR | Selector::OP_HUP)) != 0)
		    {
		        newOps = intOps;
		        sk.nio_ready_ops(newOps);
		        return (newOps & ~oldOps) != 0;
		    }

		    if (((ops & Selector::OP_READ) != 0) && ((intOps & SelectionKey::OP_ACCEPT) != 0))
		        newOps |= SelectionKey::OP_ACCEPT;

		    sk.nio_ready_ops(newOps);
		    return (newOps & ~oldOps) != 0;
		}

		bool ServerSocketChannel::translate_and_update_ready_ops(int ops, SelectionKey& sk)
		{
		    return translate_ready_ops(ops, sk.ready_ops(), sk);
		}

		bool ServerSocketChannel::translate_and_set_ready_ops(int ops, SelectionKey& sk)
		{
		    return translate_ready_ops(ops, 0, sk);
		}

		/**
		 * Translates an interest operation set into a native poll event set
		 */
		bool ServerSocketChannel::translate_and_set_interest_ops(int ops, SelectionKey& sk)
		{
		    int newOps = 0;

		    // Translate ops
		    if ((ops & SelectionKey::OP_ACCEPT) != 0)
		        newOps |= Selector::OP_READ;

		    sk.selector()->put_event_ops(sk, newOps);

		    return true;
		}

		void ServerSocketChannel::impl_configure_blocking(bool block)
		{
		    Net::configure_blocking(_fd, block);
		}

		void ServerSocketChannel::close()
		{
		    deregister();

		    if (!is_registered())
		    {
		        kill();
		    }

		    SelectableChannel::close();
		}	
	}
}


