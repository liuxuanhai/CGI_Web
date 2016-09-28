#ifndef _LCE_SOCKETCHANNEL_H_
#define _LCE_SOCKETCHANNEL_H_

#include <sys/time.h>
#include <sys/uio.h>
#include "net/socketaddress.h"
#include "net/selectablechannel.h"
#include "net/socketchannel.h"
#include "net/bytebuffer.h"
#include "net/listbytebuffer.h"
#include "net/bytechannel.h"
#include "net/net.h"

namespace lce
{
	namespace net
	{
		class SocketChannel : public SelectableChannel, public ByteChannel
		{
		public:
		    //socket error no
		    static int ERR_UNDEFINED;    
		    static int ERR_NORMAL_CLOSED;

		private:
		    // State, increases monotonically
		    static int SHUT_RD;
		    static int SHUT_WR;
		    static int SHUT_RDWR;

		    static  int ST_UNINITIALIZED;
		    static  int ST_UNCONNECTED;
		    static  int ST_PENDING;
		    static  int ST_CONNECTED;
		    static  int ST_KILLED;


		    int _fd;
		    int                    _state;
		    int                    _last_error;

		    // Binding
		    InetSocketAddress _local_address;
		    InetSocketAddress _remote_address;

		    // Input/Output open
		    bool _is_input_open;
		    bool _is_output_open;
		    bool _ready_to_connect;

		public:
		    // Constructor for normal connecting sockets
		    //
		    SocketChannel();

		    // Constructor for sockets obtained from server sockets
		    //

		    ~SocketChannel( );

		    SocketChannel& operator=(const SocketChannel& rhs);

		    void open();
		    void attach(int fd, InetSocketAddress& remote);
			
		    virtual int valid_ops();
		    virtual int read(ByteBuffer& dst);
			virtual int read(ListByteBuffer& dst );
			int read(iovec *iov, int iov_len);
		    int sync_read( ByteBuffer& dst, int timeout );
			int sync_read( ListByteBuffer& dst, int timeout );
		    int sync_read(char* buf, int buf_len, int timeout);
		    int read(char* buf, int buf_len);

			///argument:
			///time_out: in millisecond
			///return
			///0: timeout
			///-1: err
			///>0: ok (return actual read num)
			int read(char* buf, int buf_len, int timeout);

			///argument:
			///time_out: in millisecond
			///return
			///0: timeout   (buf_len return actual read)
			///-1: err
			///buf_len: ok
			int readn(char* buf, int& buf_len, int timeout);

		    int write(iovec *iov, int iov_len);
		    virtual int write( ByteBuffer& src );
			int write( ListByteBuffer& src );
		    int sync_writev(const struct iovec* vec, int count, int timeout);
		    int sync_write(char* buf, int buf_len, int timeout);
		    int sync_write( ByteBuffer& src, int timeout );   
			int sync_write( ListByteBuffer& src, int timeout );  
		    int write(char* buf, int buf_len);

		    bool ensure_read_open();
		    bool ensure_write_open();

		    bool is_bound() ;
		    InetSocketAddress local_address();
		    InetSocketAddress remote_address();

		    int bind(InetSocketAddress& local) ;
		    int bind(int port);
		    int bind(char* ip, int port);

		    bool is_connected();
		    bool is_connection_pending();
		    bool ensure_open_and_unconnected();
		    bool connect(InetSocketAddress& sa);
		    bool connect(InetSocketAddress& sa, int timeout);
		    bool connect(char* ip, int port);
		    bool connect(char* ip, int port, int timeout);    
		    bool finish_connect();

		    int shutdown(int fd, int how);

		    int shutdown_input();
		    int shutdown_output();
		    bool is_input_open();
		    bool is_onput_open();

		    virtual void close();
		    virtual int get_fd();
		    virtual void kill();
		    int   get_last_error();
		    
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
		    //
		    /**
		     * Translates native poll revent ops into a ready operation ops
		     */
		    bool translate_ready_ops(int ops, int initialOps, SelectionKey& sk) ;
		    virtual bool translate_and_update_ready_ops(int ops, SelectionKey& sk);
		    virtual bool translate_and_set_ready_ops(int ops, SelectionKey& sk);
		    virtual bool translate_and_set_interest_ops(int ops, SelectionKey& sk);


		private:

			int check_connect(int fd, bool block, bool ready);
		    virtual void impl_configure_blocking(bool block);
		public:
			void _seda_fix_connect();
		};
	}
}

#endif

