#ifndef _LCE_CHANNEL_HPP_
#define _LCE_CHANNEL_HPP_

#include "net/bytebuffer.h"
#include "net/listbytebuffer.h"

namespace lce
{
	namespace net
	{
		class ByteChannel
		{
		public:
		    ByteChannel(){}
		    virtual ~ByteChannel() {}
		    virtual int read(ByteBuffer& bb) = 0;
			virtual int read(ListByteBuffer& lbb) = 0;
		    virtual int write(ByteBuffer& bb) = 0;
		};
	
	}
}

#endif

