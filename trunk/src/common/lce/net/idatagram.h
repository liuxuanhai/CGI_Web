#ifndef _LCE_IDATAGRAM_H_
#define _LCE_IDATAGRAM_H_

#include <assert.h>
#include <stdint.h>

namespace lce
{
	namespace net
	{
		class ListByteBuffer;
		/**
		 * The datagram is the fundamental quantum in the UberMQ message kernel. All
		 * information sent between peers is in the form of a datagram.
		 */
		class IDatagram
		{
		public:
			/**
			 * Returns the type of the datagram. Intended for use with a datagram factory.
			 * @return the type value that can be used to instantiate the correct
			 * datagram implementation when reading datagrams from  a buffer.
			 */
			virtual ~IDatagram(){}
			virtual uint8_t get_datagram_type() = 0;
			
		    
		    template< class Archive >
		    void serialize( Archive& ar ){ assert(0); } 
		};	
	}
}

#endif // ifndef

