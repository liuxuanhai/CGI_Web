#ifndef _LCE_CONNECTION_INFO_H_
#define _LCE_CONNECTION_INFO_H_

#include <stdint.h>
#include <list>
#include <ext/hash_map>
#include <ext/hash_set>
#include "net/attachment.h"
#include "net/bytebuffer.h"
#include "net/listbytebuffer.h"
#include "net/imessageprocessor.h"
#include "net/connectionevent.h"
#include "net/connectioneventlistener.h"
#include "net/bytechannel.h"
#include "net/selectionkey.h"
#include "net/socketchannel.h"
#include "net/idatagram.h"
#include "net/idatagramfactory.h"

using namespace __gnu_cxx;
using namespace std;

namespace lce
{
	namespace net
	{
		class ConnectionInfo;

		typedef list<ConnectionInfo*>   ConnectionInfoList;

		class ConnectionInfo : public Attachment
		{
		     
		public:
		    enum
		    {
		        PACKET_PROCESSED_ONCE   = 100,
		    };

		    ConnectionInfo( IDatagramFactory* factory, FixedSizeAllocator* allocator );
		    ConnectionInfo( SocketChannel& channel, IMessageProcessor& p,
				IDatagramFactory* factory, FixedSizeAllocator* allocator );

		    virtual ~ConnectionInfo();

		public:
		    int   run(SelectionKey& key);
		    int   bind(IMessageProcessor& p);
		    int   open(SocketChannel& channel, IMessageProcessor& p);
		    int   open(IMessageProcessor& p); 
		    int   open();
		    
		    bool is_open();
		    void close();

		    bool ready_to_write();
		    void read(ByteChannel& channel, SelectionKey& key);
		    int   write_buf_remaining();
	            int write_buf_data_length();
		    int   write(ByteBuffer& bb);
			int   cat_write(ListByteBuffer& lbb);
			int   cpy_write(ListByteBuffer& lbb);
			int   write(IDatagram& datagram);

		    int  flush();
		    bool flushable();

		    int packet_processed_once();
		    int    packet_processed_once(int num);
		    void process_data();

		    void add_event_listener(ConnectionEventListener& l);
		    void remove_event_listener(ConnectionEventListener& l);

		    int get_id();

		    int get_timer_id();
		    int set_timer_id(int timer_id);
		    
		    SocketChannel& channel(SocketChannel& channel);
		    SocketChannel& get_channel();
		    int get_read_bytes();
		    int reset_read_bytes();    
		    int get_write_bytes();

		    static  uint32_t allocate_process_unique_id();

		private:
	           void interest_write();
		    void send_event(ConnectionEvent& event);
		    void send_event(int eventCode);

		    void pre_process_data();
		    void post_process_data();

		    void efficient_compact(ByteBuffer& bb);
			void efficient_compact(ListByteBuffer& lbb);

		private:
		    uint32_t _id;
		    int         _timer_id;
		    uint32_t _total_read_bytes;
		    uint32_t _total_write_bytes;
		    uint32_t _packet_processed_once;

		    bool _open;
		    SocketChannel _channel;
			
		    ListByteBuffer _read_buffer;
		    ListByteBuffer _write_buffer;

		    ConnectionEventListernerSet _event_handlers;

		    // statics
		    static long _next_id;

		    // the message processor. very important.
		    IMessageProcessor* _proc;

			IDatagramFactory* _datagram_factory;

		};



		inline
		int ConnectionInfo::get_id()
		{
		    return _id;
		}

		inline
		int ConnectionInfo::get_timer_id()
		{
		    return _timer_id;
		}

		inline
		int ConnectionInfo::set_timer_id(int timer_id)
		{
		    return _timer_id = timer_id;
		}

		inline
		SocketChannel& ConnectionInfo::channel(SocketChannel& channel)
		{
		    return _channel = channel;
		}
		    
		inline
		SocketChannel& ConnectionInfo::get_channel()
		{
		    return _channel;
		}

		inline
		int ConnectionInfo::get_read_bytes()
		{
		    return _total_read_bytes;
		}

		 
		inline
		int ConnectionInfo::reset_read_bytes()
		{
		    return (_total_read_bytes=0);
		}

		inline
		int ConnectionInfo::get_write_bytes()
		{
		    return _total_write_bytes;
		}

		inline
		int ConnectionInfo::write_buf_remaining()
		{
		    return _write_buffer.remaining();
		}

	        inline
		int ConnectionInfo::write_buf_data_length()
		{
		    return _write_buffer.data_length();
		}

		inline
		int    ConnectionInfo::packet_processed_once()
		{
		    return _packet_processed_once;
		}

		inline
		int    ConnectionInfo::packet_processed_once(int num)
		{
		    return _packet_processed_once = num;
		}
		    
		inline
		uint32_t ConnectionInfo::allocate_process_unique_id()
		{
		    return ++_next_id;
		}

		inline
		bool ConnectionInfo::flushable()
		{
		    return _write_buffer.has_data();
		}

		typedef hash_map<uint32_t, ConnectionInfo* > ConnectionMap;
		typedef hash_set<ConnectionInfo*> ConnectionSet;
	}
}    

namespace __gnu_cxx
{
	template<> 
	struct hash<lce::net::ConnectionInfo*>
	{
	    size_t operator()(const lce::net::ConnectionInfo* __s) const{ return  (size_t)__s;  }
	};
}



#endif

