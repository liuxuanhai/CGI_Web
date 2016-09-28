#ifndef _LCE_SERVERSOCKETCHANNEL_H_
#define _LCE_SERVERSOCKETCHANNEL_H_
#include "net/selectablechannel.h"
#include "net/socketaddress.h"
#include "net/net.h"

namespace lce
{
	namespace net
	{
		class SocketChannel;
		class ServerSocketChannel: public SelectableChannel
		{
		private:
		    int _fd;
		    // Channel state, increases monotonically
		    static  int ST_UNINITIALIZED;
		    static  int ST_INUSE;
		    static  int ST_KILLED;

		    int _state;

		    // Binding
		    InetSocketAddress _local_address; // null => unbound

		public:
		    enum
		    {
		        DEF_BACKLOG_LENGTH = 50,
		    };

		    ServerSocketChannel();
		    ServerSocketChannel(int fd);
		    virtual ~ServerSocketChannel(){}

		    int valid_ops() ;
		    bool is_bound();
		    InetSocketAddress local_address();
		    int  bind(InetSocketAddress& local, int backlog = DEF_BACKLOG_LENGTH);
		    int  bind(int port, int backlog = DEF_BACKLOG_LENGTH);
		    int  bind(char* ip, int port, int backlog = DEF_BACKLOG_LENGTH);
		    SocketChannel* accept();
		    int accept(SocketChannel& sc);

		    void close();
		    void kill();

		    int get_fd();

		    static  int listen(int fd, int backlog);

		    // socket options
		    // SO_KEEPALIVE
		    bool keep_alive();
		    void keep_alive(bool b);

		    // SO_LINGER
		    int linger();
		    void linger(int n);

		    // SO_OOBINLINE
		    bool out_of_band_inline();
		    void out_of_band_inline(bool b);

		    // SO_RCVBUF
		    int recv_buf_size();
		    void recv_buf_size(int n);

		    // SO_SNDBUF
		    int send_buf_size();
		    void send_buf_size(int n);

		    // SO_REUSEADDR
		    bool reuse_addr() ;
		    void reuse_addr(bool b);

		    //  SO_SNDTIMEO
		    int send_timeout();
		    void  send_timeout(int seconds);
		    
		    bool tcp_nodelay();
		    void tcp_nodelay(bool b);
		    
		    bool translate_ready_ops(int ops, int initialOps,SelectionKey& sk);

		    bool translate_and_update_ready_ops(int ops, SelectionKey& sk);
		    bool translate_and_set_ready_ops(int ops, SelectionKey& sk) ;

		    /**
		     * Translates an interest operation set into a native poll event set
		     */
		    bool translate_and_set_interest_ops(int ops, SelectionKey& sk) ;

		    void impl_configure_blocking(bool block);

		};
	}
}

#endif
