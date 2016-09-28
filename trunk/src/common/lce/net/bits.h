#ifndef _LCE_BITS_H_
#define _LCE_BITS_H_

#include <stdint.h>
#include <string>
#include "net/listbuffer.h"

using namespace std;

namespace lce
{
	namespace net
	{
		typedef  char byte;

		class ByteBuffer;
		class ByteOrder;
		class ListByteBuffer;

		class Bits
		{
		private:
		    static ByteOrder _byte_order;

		public:
		    Bits() ;


		    // -- Swapping --

		    static short swap(short x) ;

		    static char swap(char x) ;

		    static int swap(int x) ;

		    // -- get/put short --

		    static  short make_short(byte b1, byte b0) ;

		    static short get_shortL(ByteBuffer& bb, int bi) ;

			static short get_shortL(ListByteBuffer& lbb, ListBuffer::Position p);
			
		    static short get_shortB(ByteBuffer& bb, int bi);

			static short get_shortB(ListByteBuffer& lbb, ListBuffer::Position p);


		    static short get_short(ByteBuffer& bb, int bi, bool bigEndian);

			static short get_short(ListByteBuffer& lbb, ListBuffer::Position p, bool bigEndian);

		    static char short1(short x) ;
		    static char short0(short x) ;

		    static void put_shortL(ByteBuffer& bb, int bi, short x);

			static void put_shortL(ListByteBuffer& lbb, ListBuffer::Position p, short x);


		    static void put_shortB(ByteBuffer& bb, int bi, short x);

			static void put_shortB(ListByteBuffer& lbb, ListBuffer::Position p, short x);



		    static void put_short(ByteBuffer& bb, int bi, short x, bool bigEndian) ;

			static void put_short(ListByteBuffer& lbb, ListBuffer::Position bi, short x, bool bigEndian) ;




		    // -- get/put int --

		    static  int make_int(byte b3, byte b2, byte b1, byte b0) ;
		    static int get_intL(ByteBuffer& bb, int bi) ;
			static int get_intL(ListByteBuffer& lbb, ListBuffer::Position p) ;
		    static int get_intB(ByteBuffer& bb, int bi);
			static int get_intB(ListByteBuffer& lbb, ListBuffer::Position p);
		    static int get_int(ByteBuffer& bb, int bi, bool bigEndian) ;
			static int get_int(ListByteBuffer& lbb, ListBuffer::Position bi, bool bigEndian) ;

		    static char int3(int x);
		    static char int2(int x);
		    static char int1(int x);
		    static char int0(int x);

		    static void put_intL(ByteBuffer& bb, int bi, int x) ;
			static void put_intL(ListByteBuffer& lbb, ListBuffer::Position p, int x) ;
		    static void put_intB(ByteBuffer& bb, int bi, int x) ;
			static void put_intB(ListByteBuffer& lbb, ListBuffer::Position p, int x) ;
		    static void put_int(ByteBuffer& bb, int bi, int x, bool bigEndian) ;
			static void put_int(ListByteBuffer& lbb, ListBuffer::Position p, int x, bool bigEndian) ;

		   //get/put long(int64)
		    static int64_t make_long(byte b7, byte b6, byte b5, byte b4,
					                byte b3, byte b2, byte b1, byte b0);
		    static int64_t get_longL(ByteBuffer& bb, int bi) ;
			static int64_t get_longL(ListByteBuffer& lbb, ListBuffer::Position p) ;
		    static int64_t get_longB(ByteBuffer& bb, int bi);
			static int64_t get_longB(ListByteBuffer& lbb, ListBuffer::Position p);
		    static int64_t get_long(ByteBuffer& bb, int bi, bool bigEndian);
			static int64_t get_long(ListByteBuffer& lbb, ListBuffer::Position p, bool bigEndian);
		    
		    static char  long7(int64_t x);
		    static char  long6(int64_t x);
		    static char  long5(int64_t x);
		    static char  long4(int64_t x);
		    static char  long3(int64_t x);
		    static char  long2(int64_t x);
		    static char  long1(int64_t x);
		    static char  long0(int64_t x);

		    static void put_longL(ByteBuffer& bb, int bi, int64_t x) ;
			static void put_longL(ListByteBuffer& lbb, ListBuffer::Position p, int64_t x) ;
		    static void put_longB(ByteBuffer& bb, int bi, int64_t x) ;
			static void put_longB(ListByteBuffer& lbb, ListBuffer::Position p, int64_t x) ;
		    static void put_long(ByteBuffer& bb, int bi, int64_t x, bool bigEndian) ;
			static void put_long(ListByteBuffer& lbb, ListBuffer::Position p, int64_t x, bool bigEndian) ;
			// -- get/put float --

		    static float get_floatL(ByteBuffer& bb, int bi) ;


			static float get_floatL(ListByteBuffer& lbb, ListBuffer::Position p) ;


		    static float get_floatB(ByteBuffer& bb, int bi) ;


			static float get_floatB(ListByteBuffer& lbb, ListBuffer::Position p) ;



		    static float get_float(ByteBuffer& bb, int bi, bool bigEndian) ;


			static float get_float(ListByteBuffer& lbb, ListBuffer::Position p, bool bigEndian) ;



		    static void put_floatL(ByteBuffer& bb, int bi, float x) ;


			static void put_floatL(ListByteBuffer& lbb, ListBuffer::Position p, float x) ;



		    static void put_floatB(ByteBuffer& bb, int bi, float x) ;


			static void put_floatB(ListByteBuffer& lbb, ListBuffer::Position p, float x) ;


		    static void put_float(ByteBuffer& bb, int bi, float x, bool bigEndian) ;


			static void put_float(ListByteBuffer& lbb, ListBuffer::Position p, float x, bool bigEndian) ;

		    static ByteOrder byte_order() ;
		};

		inline
		Bits::Bits()
		{

		}


		// -- Swapping --

		inline
		short  Bits::swap(short x) {
		    return (short)((x << 8) |
		                   ((x >> 8) & 0xff));
		}

		inline
		char  Bits::swap(char x) {
		    return (char)((x << 8) |
		                  ((x >> 8) & 0xff));
		}

		inline
		int  Bits::swap(int x) {
		    return (int)((swap((short)x) << 16) |
		                 (swap((short)(x >> 16)) & 0xffff));
		}


		// -- get/put short --
		inline
		short  Bits::make_short(byte b1, byte b0) {
		    return (short)((b1 << 8) | (b0 & 0xff));
		}

		inline
		int  Bits::make_int(byte b3, byte b2, byte b1, byte b0) {
		    return (int)((((b3 & 0xff) << 24) |
		                  ((b2 & 0xff) << 16) |
		                  ((b1 & 0xff) <<  8) |
		                  ((b0 & 0xff) <<  0)));
		}

		inline
		int64_t Bits::make_long(byte b7, byte b6, byte b5, byte b4,
					 byte b3, byte b2, byte b1, byte b0)
		{
		return ((((int64_t)b7 & 0xff) << 56) |
			(((int64_t)b6 & 0xff) << 48) |
			(((int64_t)b5 & 0xff) << 40) |
			(((int64_t)b4 & 0xff) << 32) |
			(((int64_t)b3 & 0xff) << 24) |
			(((int64_t)b2 & 0xff) << 16) |
			(((int64_t)b1 & 0xff) <<  8) |
			(((int64_t)b0 & 0xff) <<  0));
		}

		inline
		byte  Bits::short1(short x) { return (byte)(x >> 8); }

		inline
		byte  Bits::short0(short x) { return (byte)(x >> 0); }

		inline
		char  Bits::int3(int x) { return (byte)(x >> 24); }

		inline
		char  Bits::int2(int x) { return (byte)(x >> 16); }

		inline
		char  Bits::int1(int x) { return (byte)(x >>  8); }

		inline
		char  Bits::int0(int x) { return (byte)(x >>  0); }


		inline
		char  Bits::long7(int64_t x) { return (byte)(x >> 56); }
		inline
		char  Bits::long6(int64_t x) { return (byte)(x >> 48); }
		inline
		char  Bits::long5(int64_t x) { return (byte)(x >> 40); }
		inline
		char  Bits::long4(int64_t x) { return (byte)(x >> 32); }
		inline
		char  Bits::long3(int64_t x) { return (byte)(x >> 24); }
		inline
		char  Bits::long2(int64_t x) { return (byte)(x >> 16); }
		inline
		char  Bits::long1(int64_t x) { return (byte)(x >>  8); }
		inline
		char  Bits::long0(int64_t x) { return (byte)(x >>  0); }	
	}
}
#endif

