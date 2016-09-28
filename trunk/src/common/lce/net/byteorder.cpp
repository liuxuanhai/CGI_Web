#include "net/bits.h"
#include "net/byteorder.h"

namespace lce
{
	namespace net
	{
		ByteOrder::ByteOrder(string name)
		{
		    this->name = name;
		}


		ByteOrder ByteOrder::native_order()
		{
		    return Bits::byte_order();
		}


		ByteOrder ByteOrder::big_endian("BIG_ENDIAN");
		ByteOrder ByteOrder::little_endian("LITTLE_ENDIAN");
	}
}

