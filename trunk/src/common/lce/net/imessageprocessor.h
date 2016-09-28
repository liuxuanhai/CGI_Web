#ifndef _LCE_IMESSAGE_PROCESSOR_H_
#define _LCE_IMESSAGE_PROCESSOR_H_

#include <ext/hash_map>
#include <ext/hash_set>

#include "net/idatagram.h"

using namespace __gnu_cxx;

namespace lce
{
	namespace net
	{
		class ConnectionInfo;
		class ByteBuffer;
		class IMessageProcessor
		{
		public:
		    IMessageProcessor(){}
		    virtual ~IMessageProcessor(){}
		    virtual void accept(ConnectionInfo& conn) = 0;
		    virtual void remove(ConnectionInfo& conn) = 0;

		    virtual void process_input( ConnectionInfo& conn, IDatagram& gram ) = 0;
			virtual void process_input( ConnectionInfo& conn, ListByteBuffer* lbb ){}
		    virtual void process_output() = 0;
		    //virtual int frame(ByteBuffer& bb)=0;
		};

	}
}

namespace __gnu_cxx
{
	template<> 
	struct hash<lce::net::IMessageProcessor*>
	{
	    size_t operator()(const lce::net::IMessageProcessor* __s) const
	        { return  (size_t)__s;  }



	};
}

#endif

