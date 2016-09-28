#ifndef _LCE_BYTEORDER_H_
#define _LCE_BYTEORDER_H_

#include "net/bits.h"

namespace lce
{
	namespace net
	{
		class ByteOrder
		{
		public:
		    string name;
		public:
		    ByteOrder(string name) ;

		    static ByteOrder big_endian;
		    static ByteOrder little_endian;

		    static ByteOrder native_order();
		};	
	}
}
#endif

