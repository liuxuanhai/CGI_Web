#include "net/byteorder.h"
#include "net/bytebuffer.h"
#include "net/listbytebuffer.h"
#include "net/listbuffer.h"
#include "net/bits.h"

namespace lce
{
	namespace net
	{
		ByteOrder Bits::_byte_order("LITTLE_ENDIAN");

		short  Bits::get_shortL(ByteBuffer& bb, int bi) 
		{
		    return make_short(bb._get(bi + 1),
		                     bb._get(bi + 0));
		}

		short Bits::get_shortL(ListByteBuffer& lbb, ListBuffer::Position p)
		{
			return make_short(lbb._get(p+1), 
							lbb._get(p+0));
		}

		short  Bits::get_shortB(ByteBuffer& bb, int bi) 
		{
		    return make_short(bb._get(bi + 0),
		                     bb._get(bi + 1));
		}


		short  Bits::get_shortB(ListByteBuffer& lbb, ListBuffer::Position p) 
		{
		    return make_short(lbb._get(p + 0),
		                     lbb._get(p + 1));
		}


		short  Bits::get_short(ByteBuffer& bb, int bi, bool bigEndian) 
		{
		    return (bigEndian ? get_shortB(bb, bi) : get_shortL(bb, bi));
		}


		short  Bits::get_short(ListByteBuffer& lbb, ListBuffer::Position p, bool bigEndian) 
		{
		    return (bigEndian ? get_shortB(lbb, p) : get_shortL(lbb, p));
		}

		void  Bits::put_shortL(ByteBuffer& bb, int bi, short x) 
		{
		    bb._put(bi + 0, short0(x));
		    bb._put(bi + 1, short1(x));
		}


		void  Bits::put_shortL(ListByteBuffer& lbb, ListBuffer::Position p, short x) 
		{
		    lbb._put(p + 0, short0(x));
		    lbb._put(p + 1, short1(x));
		}

		void  Bits::put_shortB(ByteBuffer& bb, int bi, short x) 
		{
		    bb._put(bi + 0, short1(x));
		    bb._put(bi + 1, short0(x));
		}


		void  Bits::put_shortB(ListByteBuffer& lbb, ListBuffer::Position p, short x) 
		{
		    lbb._put(p + 0, short1(x));
		    lbb._put(p + 1, short0(x));
		}

		void  Bits::put_short(ByteBuffer& bb, int bi, short x, bool bigEndian) 
		{
		    if (bigEndian)
		        put_shortB(bb, bi, x);
		    else
		        put_shortL(bb, bi, x);
		}

		void  Bits::put_short(ListByteBuffer& lbb, ListBuffer::Position p, short x, bool bigEndian) 
		{
		    if (bigEndian)
		        put_shortB(lbb, p, x);
		    else
		        put_shortL(lbb, p, x);
		}

		// -- get/put int --

		int  Bits::get_intL(ByteBuffer& bb, int bi) 
		{
		    return make_int(bb._get(bi + 3),
		                   bb._get(bi + 2),
		                   bb._get(bi + 1),
		                   bb._get(bi + 0));
		}

		int  Bits::get_intL(ListByteBuffer& lbb, ListBuffer::Position p) 
		{
		    return make_int(lbb._get(p + 3),
		                   lbb._get(p + 2),
		                   lbb._get(p + 1),
		                   lbb._get(p + 0));
		}

		int  Bits::get_intB(ByteBuffer& bb, int bi) 
		{
		    return make_int(bb._get(bi + 0),
		                   bb._get(bi + 1),
		                   bb._get(bi + 2),
		                   bb._get(bi + 3));
		}

		int  Bits::get_intB(ListByteBuffer& lbb, ListBuffer::Position p) 
		{
		    return make_int(lbb._get(p + 0),
		                   lbb._get(p + 1),
		                   lbb._get(p + 2),
		                   lbb._get(p + 3));
		}
		
		int  Bits::get_int(ByteBuffer& bb, int bi, bool bigEndian) 
		{
		    return (bigEndian ? get_intB(bb, bi) : get_intL(bb, bi));
		}

		int  Bits::get_int(ListByteBuffer& lbb, ListBuffer::Position p, bool bigEndian) 
		{
		    return (bigEndian ? get_intB(lbb, p) : get_intL(lbb, p));
		}

		void  Bits::put_intL(ByteBuffer& bb, int bi, int x) 
		{
		    bb._put(bi + 3, int3(x));
		    bb._put(bi + 2, int2(x));
		    bb._put(bi + 1, int1(x));
		    bb._put(bi + 0, int0(x));
		}

		void  Bits::put_intL(ListByteBuffer& lbb, ListBuffer::Position p, int x) 
		{
		    lbb._put(p + 3, int3(x));
		    lbb._put(p + 2, int2(x));
		    lbb._put(p + 1, int1(x));
		    lbb._put(p + 0, int0(x));
		}

		void  Bits::put_intB(ByteBuffer& bb, int bi, int x) {
		    bb._put(bi + 0, int3(x));
		    bb._put(bi + 1, int2(x));
		    bb._put(bi + 2, int1(x));
		    bb._put(bi + 3, int0(x));
		}

		void  Bits::put_intB(ListByteBuffer& lbb, ListBuffer::Position p, int x) {
		    lbb._put(p + 0, int3(x));
		    lbb._put(p + 1, int2(x));
		    lbb._put(p + 2, int1(x));
		    lbb._put(p + 3, int0(x));
		}

		void  Bits::put_int(ByteBuffer& bb, int bi, int x, bool bigEndian) 
		{
		    if (bigEndian)
		        put_intB(bb, bi, x);
		    else
		        put_intL(bb, bi, x);
		}

		void  Bits::put_int(ListByteBuffer& lbb, ListBuffer::Position p, int x, bool bigEndian) 
		{
		    if (bigEndian)
		        put_intB(lbb, p, x);
		    else
		        put_intL(lbb, p, x);
		}

		 // -- get/put long --

		int64_t  Bits::get_longL(ByteBuffer& bb, int bi) 
		{
		return make_long(bb._get(bi + 7),
				bb._get(bi + 6),
				bb._get(bi + 5),
				bb._get(bi + 4),
				bb._get(bi + 3),
				bb._get(bi + 2),
				bb._get(bi + 1),
				bb._get(bi + 0));
		}


		int64_t  Bits::get_longL(ListByteBuffer& lbb, ListBuffer::Position p) 
		{
		return make_long(lbb._get(p + 7),
				lbb._get(p + 6),
				lbb._get(p + 5),
				lbb._get(p + 4),
				lbb._get(p + 3),
				lbb._get(p + 2),
				lbb._get(p + 1),
				lbb._get(p + 0));
		}

		int64_t  Bits::get_longB(ByteBuffer& bb, int bi) 
		{
		return make_long(bb._get(bi + 0),
				bb._get(bi + 1),
				bb._get(bi + 2),
				bb._get(bi + 3),
				bb._get(bi + 4),
				bb._get(bi + 5),
				bb._get(bi + 6),
				bb._get(bi + 7));
		}


		int64_t  Bits::get_longB(ListByteBuffer& lbb, ListBuffer::Position p) 
		{
		return make_long(lbb._get(p + 0),
				lbb._get(p + 1),
				lbb._get(p + 2),
				lbb._get(p + 3),
				lbb._get(p + 4),
				lbb._get(p + 5),
				lbb._get(p + 6),
				lbb._get(p + 7));
		}


		int64_t  Bits::get_long(ByteBuffer& bb, int bi, bool bigEndian) 
		{
		    return (bigEndian ? get_longB(bb, bi) : get_longL(bb, bi));
		}

		int64_t  Bits::get_long(ListByteBuffer& lbb, ListBuffer::Position p, bool bigEndian) 
		{
		    return (bigEndian ? get_longB(lbb, p) : get_longL(lbb, p));
		}


		void  Bits::put_longL(ByteBuffer& bb, int bi, int64_t x) 
		{
		    bb._put(bi + 7, long7(x));
		    bb._put(bi + 6, long6(x));
		    bb._put(bi + 5, long5(x));
		    bb._put(bi + 4, long4(x));
		    bb._put(bi + 3, long3(x));
		    bb._put(bi + 2, long2(x));
		    bb._put(bi + 1, long1(x));
		    bb._put(bi + 0, long0(x));
		}


		void  Bits::put_longL(ListByteBuffer& lbb, ListBuffer::Position p, int64_t x) 
		{
		    lbb._put(p + 7, long7(x));
		    lbb._put(p + 6, long6(x));
		    lbb._put(p + 5, long5(x));
		    lbb._put(p + 4, long4(x));
		    lbb._put(p + 3, long3(x));
		    lbb._put(p + 2, long2(x));
		    lbb._put(p + 1, long1(x));
		    lbb._put(p + 0, long0(x));
		}


		void  Bits::put_longB(ByteBuffer& bb, int bi, int64_t x)
		{
		    bb._put(bi + 0, long7(x));
		    bb._put(bi + 1, long6(x));
		    bb._put(bi + 2, long5(x));
		    bb._put(bi + 3, long4(x));
		    bb._put(bi + 4, long3(x));
		    bb._put(bi + 5, long2(x));
		    bb._put(bi + 6, long1(x));
		    bb._put(bi + 7, long0(x));
		}



		void  Bits::put_longB(ListByteBuffer& lbb, ListBuffer::Position p, int64_t x)
		{
		    lbb._put(p + 0, long7(x));
		    lbb._put(p + 1, long6(x));
		    lbb._put(p + 2, long5(x));
		    lbb._put(p + 3, long4(x));
		    lbb._put(p + 4, long3(x));
		    lbb._put(p + 5, long2(x));
		    lbb._put(p + 6, long1(x));
		    lbb._put(p + 7, long0(x));
		}

		void  Bits::put_long(ByteBuffer& bb, int bi, int64_t x, bool bigEndian) 
		{
		    if (bigEndian)
		        put_longB(bb, bi, x);
		    else
		        put_longL(bb, bi, x);
		}


		void  Bits::put_long(ListByteBuffer& lbb, ListBuffer::Position p, int64_t x, bool bigEndian) 
		{
		    if (bigEndian)
		        put_longB(lbb, p, x);
		    else
		        put_longL(lbb, p, x);
		}

		// -- get/put float --

		float  Bits::get_floatL(ByteBuffer& bb, int bi) {
		//return Float.intBitsToFloat(getIntL(bb, bi));
		    return 0;
		}

		float  Bits::get_floatL(ListByteBuffer& lbb, ListBuffer::Position p) {
		//return Float.intBitsToFloat(getIntL(bb, bi));
		    return 0;
		}


		float  Bits::get_floatB(ByteBuffer& bb, int bi) {
		//return Float.intBitsToFloat(getIntB(bb, bi));
		    return 0;
		}


		float  Bits::get_floatB(ListByteBuffer& lbb, ListBuffer::Position p) {
		//return Float.intBitsToFloat(getIntB(bb, bi));
		    return 0;
		}

		float  Bits::get_float(ByteBuffer& bb, int bi, bool bigEndian) {
		//return (bigEndian ? getFloatB(bb, bi) : getFloatL(bb, bi));
		    return 0;
		}


		float  Bits::get_float(ListByteBuffer& lbb, ListBuffer::Position p, bool bigEndian) {
		//return (bigEndian ? getFloatB(bb, bi) : getFloatL(bb, bi));
		    return 0;
		}

		void  Bits::put_floatL(ByteBuffer& bb, int bi, float x) {
		//putIntL(bb, bi, Float.floatToRawIntBits(x));
		    return ;
		}


		void  Bits::put_floatL(ListByteBuffer& lbb, ListBuffer::Position p, float x) {
		//putIntL(bb, bi, Float.floatToRawIntBits(x));
		    return ;
		}

		void  Bits::put_floatB(ByteBuffer& bb, int bi, float x) {
		//putIntB(bb, bi, Float.floatToRawIntBits(x));
		    return ;
		}


		void  Bits::put_floatB(ListByteBuffer& lbb, ListBuffer::Position p, float x) {
		//putIntB(bb, bi, Float.floatToRawIntBits(x));
		    return ;
		}

		void  Bits::put_float(ByteBuffer& bb, int bi, float x, bool bigEndian) {
		    if (bigEndian)
		        put_floatB(bb, bi, x);
		    else
		        put_floatL(bb, bi, x);
		}


		void  Bits::put_float(ListByteBuffer& lbb, ListBuffer::Position p, float x, bool bigEndian) {
		    if (bigEndian)
		        put_floatB(lbb, p, x);
		    else
		        put_floatL(lbb, p, x);
		}

		ByteOrder  Bits::byte_order()
		{
		    unsigned int order = 0x01020304;
		    char* p   = (char*)&order;
		    switch (*p)
		    {
		    case 0x01:
		        _byte_order = ByteOrder::big_endian;     break;
		    case 0x04:
		        _byte_order = ByteOrder::little_endian;  break;
		    default:
		        break;

		    }
		    return _byte_order;

		}

	}
}

