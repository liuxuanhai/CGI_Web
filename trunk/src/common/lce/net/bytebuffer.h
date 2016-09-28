#ifndef _LCE_BYTEBUFFER_H_
#define _LCE_BYTEBUFFER_H_

#include <string>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <cstdio>
#include <new>
#include <iostream>
#include <iomanip>
#include <string>
#include "net/byteorder.h"
#include "net/bits.h"
#include "net/buffer.h"

using namespace std;

namespace lce
{
	namespace net
	{
		typedef char byte;
		class ByteOrder;
		class Bits;
		class ByteBuffer : public Buffer
		{
		public:
			typedef int Position;
		    // These fields are declared here rather than in Heap-X-Buffer in order to
		    // reduce the number of virtual method invocations needed to access these
		    // values, which is especially costly when coding small buffers.
		    //
		    char* hb;       // Non-null only for heap buffers
		    int offset;

		    bool _big_endian;
		    bool _native_byteorder;
		    bool _is_direct_allocate;

		public:
		    // Creates a new buffer with the given mark, position, limit, capacity,
		    // backing array, and array offset
		    //
		    ByteBuffer();
		    ByteBuffer(int mark, int pos, int lim, int cap, char* hb, int offset);
		    ByteBuffer(char* hb, int lim, int cap);
		    ByteBuffer(int cap);
		    ~ByteBuffer();

		    // rea mem addr tools
		    const char* address() const {return hb + offset;}
		    char* address(){return hb + offset;}
		    char* array() ;
		    int array_offset() ;
		    int ix(int i) const;

		    //byte order
		    ByteOrder order();
		    ByteBuffer& order(ByteOrder bo);

		    // slice and duplicate ,  only manager data created, keep real memdata unchanged
		    // while, copyto,  real memcpy form one (hb, offset) to another (hb, offset)
		    ByteBuffer slice();
		    ByteBuffer&  slice(ByteBuffer& bb);
		    ByteBuffer duplicate() ;
		    ByteBuffer&  duplicate(ByteBuffer& bb);
			ssize_t copy_to(ByteBuffer& bb) const;

			//move data [position, limit]  to [0, remaining]
			ByteBuffer& compact() ;

			//absolute addr，  read & write, 
		    byte _get(int i) ;
		    void _put(int i, byte b);

		    //relative addr， read  & write
		    byte get() ;
		    byte get(int i) const;
		    ByteBuffer& get(byte* dst, int dstLength, int offset, int length) ;
		    ByteBuffer& get(byte* dst, int dstLength, int length) ;
		    ByteBuffer& get(string& dst, int offset, int length);
		    ByteBuffer& get(string& dst, int length);
		    ByteBuffer& get_asciiz(char* dst,int dstLength);
		    ByteBuffer& put(byte x) ;
		    ByteBuffer& put(int i, byte x) ;
		    ByteBuffer& put(const byte* src, int srcLength, int offset, int length) ;
		    ByteBuffer& put(const byte* src, int srcLength, int length) ;
		    ByteBuffer& put(const string& src, int offset) ;
		    ByteBuffer& put(const string& src) ;
		    ByteBuffer& put(ByteBuffer& src);
		    ByteBuffer& put_asciiz(const byte* src, int srcLength);

		    // short
		    short get_short() ;
		    short get_short(int i) ;
		    ByteBuffer& put_short(short x) ;
		    ByteBuffer& put_short(int i, short x) ;

		    // int
		    int get_int() ;
		    int get_int(int i) ;
		    ByteBuffer& put_int(int x) ;
		    ByteBuffer& put_int(int i, int x);

		    //long (int64)
		    int64_t get_long();
		    int64_t get_long(int i);
		    ByteBuffer& put_long(int64_t x);
		    ByteBuffer& put_long(int i, int64_t x);
		    
		    // float
		    float get_float() ;
		    float get_float(int i) ;
		    ByteBuffer& put_float(float x);
		    ByteBuffer& put_float(int i, float x);

		    //dump out
		    void dump(std::ostream& dumped_stream) const;
		    int hash_code() ;
		    static void* arraycopy(byte* src, int  srcPos, byte* dest, int destPos, int length);
		};


		inline
		ByteBuffer::ByteBuffer()
		        :Buffer(-1,0,0,0)
		{
			this->hb = NULL;
		    this->offset = 0;
		}

		inline
		ByteBuffer::ByteBuffer(int mark, int pos, int lim, int cap,	char* hb, int offset)
		        :Buffer(mark, pos, lim, cap)
		{
		    _is_direct_allocate = false;
		    this->hb = hb;
		    this->offset = offset;

		    _big_endian	= true;
		    _native_byteorder	= (Bits::byte_order().name== ByteOrder::big_endian.name);
		}

		inline
		ByteBuffer::ByteBuffer(char* hb, int lim, int cap)
		        :Buffer(-1, 0, lim, cap)
		{
		    _is_direct_allocate = false;
		    this->hb = hb;
		    this->offset = 0;

		    _big_endian	= true;
		    _native_byteorder	= (Bits::byte_order().name== ByteOrder::big_endian.name);
		}

		inline
		ByteBuffer::ByteBuffer(int cap)
		        :Buffer(-1, 0, cap, cap)
		{
		    this->hb = new char[cap];
		    this->_is_direct_allocate = true;
		    this->offset = 0;

		    _big_endian	= true;
		    _native_byteorder	= (Bits::byte_order().name== ByteOrder::big_endian.name);
		}

		inline
		ByteBuffer::~ByteBuffer()
		{
		    if(_is_direct_allocate)
		    {
		        delete[] this->hb;
		        this->hb = NULL;
		    }

		}

		inline
		void ByteBuffer::dump(std::ostream& dumped_stream) const
		{
		    const int BYTES_INPUT_PER_LINE = 16;

		    unsigned char uchChar = '\0';
		    char achTextVer[BYTES_INPUT_PER_LINE + 1];

		    int i;

		    dumped_stream.fill('0');

		    int lines = limit()/ BYTES_INPUT_PER_LINE;
		    for (i = 0; i < lines; i++)
		    {
		        int  j;

		        for (j = 0 ; j < BYTES_INPUT_PER_LINE; j++)
		        {
		            uchChar = (unsigned char) get((i << 4) + j);
		            dumped_stream<<setw(2)<<hex<<(int)uchChar<<" ";

		            if (j == 7)
		            {
		                dumped_stream<<" ";
		            }
		            achTextVer[j] = isprint(uchChar) ? uchChar : '.';

		        }

		        achTextVer[j] = 0;
		        dumped_stream<<" "<<achTextVer<<endl;

		    }

		    if ( limit() % BYTES_INPUT_PER_LINE)
		    {
		        for (i = 0 ; i <  limit() % BYTES_INPUT_PER_LINE; i++)
		        {
		            uchChar = (unsigned char) get(limit() - limit() % BYTES_INPUT_PER_LINE + i);
		            dumped_stream<<setw(2)<<hex<<(int)uchChar<<" ";

		            if (i == 7)
		            {
		                dumped_stream<<" ";
		            }

		            achTextVer[i] = isprint (uchChar) ? uchChar : '.';
		        }

		        for (i = limit() % BYTES_INPUT_PER_LINE; i < BYTES_INPUT_PER_LINE; i++)
		        {
		            dumped_stream<<"   ";
		            if (i == 7)
		            {
		                dumped_stream<<" ";
		            }
		            achTextVer[i] = ' ';
		        }

		        achTextVer[i] = 0;
		        dumped_stream<<" "<<achTextVer<<endl;
		    }

		    return ;
		}

		inline
		void* ByteBuffer::arraycopy(byte* src,  int  srcPos,
		                            byte* dest, int destPos,
		                            int length)
		{
		    byte*  _src = src+srcPos;
		    byte* _dest = dest+destPos;

		    return memmove(_dest, _src, length);

		}
		/**
		 * Returns the byte array that backs this
		 * buffer&nbsp;&nbsp;<i>(optional operation)</i>.
		 *
		 * <p> Modifications to this buffer's content will cause the returned
		 * array's content to be modified, and vice versa.
		 *
		 * <p> Invoke the {@link #hasArray hasArray} method before invoking this
		 * method in order to ensure that this buffer has an accessible backing
		 * array.  </p>
		 *
		 * @return  The array that backs this buffer
		 *
		 * @throws  ReadOnlyBufferException
		 *          If this buffer is backed by an array but is read-only
		 *
		 * @throws  UnsupportedOperationException
		 *          If this buffer is not backed by an accessible array
		 */

		inline
		char* ByteBuffer::array()
		{
		    if (hb == NULL)
		        throw  UnsupportedOperationException();

		    return hb;
		}

		inline
		int ByteBuffer::array_offset()
		{
		    if (hb == NULL)
		        throw  UnsupportedOperationException();

		    return offset;
		}

		inline
		int ByteBuffer::hash_code()
		{
		    int h = 1;
		    int p = position();
		    for (int i = limit() - 1; i >= p; i--)
		        h = 31 * h + (int)get(i);
		    return h;
		}




		/**
		 * Retrieves this buffer's byte order.
		 *
		 * <p> The byte order is used when reading or writing multibyte values, and
		 * when creating buffers that are views of this byte buffer.  The order of
		 * a newly-created byte buffer is always {@link ByteOrder#BIG_ENDIAN
		 * BIG_ENDIAN}.  </p>
		 *
		 * @return  This buffer's byte order
		 */

		inline
		ByteOrder ByteBuffer::order()
		{
		    return _big_endian ? ByteOrder::big_endian: ByteOrder::little_endian;
		}

		/**
		 * Modifies this buffer's byte order.  </p>
		 *
		 * @param  bo
		 *         The new byte order,
		 *         either {@link ByteOrder#BIG_ENDIAN BIG_ENDIAN}
		 *         or {@link ByteOrder#LITTLE_ENDIAN LITTLE_ENDIAN}
		 *
		 * @return  This buffer
		 */

		inline
		ByteBuffer& ByteBuffer::order(ByteOrder bo)
		{
		    _big_endian = (bo.name == ByteOrder::big_endian.name);
		    _native_byteorder = (_big_endian == (Bits::byte_order().name == ByteOrder::big_endian.name));
		    return *this;
		}



		inline
		ByteBuffer ByteBuffer::slice()
		{
		    ByteBuffer bb(-1,
		                  0,
		                  this->remaining(),
		                  this->remaining(),
		                  this->hb,
		                  this->position() + offset);
		    return bb;
		}


		inline
		ByteBuffer ByteBuffer::duplicate()
		{
		    ByteBuffer bb(this->mark_value(),
		                   this->position(),
		                   this->limit(),
		                   this->capacity(),
		                   this->hb,
		                   offset);
		    return bb;
		}

		inline
		ssize_t ByteBuffer::copy_to(ByteBuffer& bb) const
		{
			int pos = position();
			int lim = limit();

			int rem = (pos <= lim ? lim - pos : 0);
			
			if (rem == 0)
			{
				return 0;
			}

			int other_pos = bb.position();
			int other_lim = bb.limit();

			int other_rem = (other_pos <= other_lim ? other_lim - other_pos : 0);

			if (other_rem < rem)
			{
				return 0;
			}
			
			memcpy(bb.address() + other_pos, address() + pos, rem);

		//	bb.position(other_pos + rem);
			try
			{
				//bb.limit(other_lim + rem);
				bb.limit(other_pos + rem);
			}
			catch(std::exception&)
			{
				return -1;
			}
			
			return static_cast<ssize_t>(rem);
		}

		inline
		ByteBuffer& ByteBuffer::slice(ByteBuffer& bb)
		{
		    return *(new(&bb)ByteBuffer(
		                 -1,
		                 0,
		                 this->remaining(),
		                 this->remaining(),
		                 this->hb,
		                 this->position() + offset));
		}


		inline
		ByteBuffer& ByteBuffer::duplicate(ByteBuffer& bb)
		{
		    return *(new(&bb)ByteBuffer(
		                 this->mark_value(),
		                 this->position(),
		                 this->limit(),
		                 this->capacity(),
		                 this->hb,
		                 offset));
		}


		inline
		int ByteBuffer::ix(int i) const
		{
		    return i + offset;
		}


		inline
		char ByteBuffer::get()
		{
		    return hb[ix(next_get_index())];
		}


		inline
		char ByteBuffer::get(int i) const
		{
		    return hb[ix(check_index(i))];
		}


		inline
		ByteBuffer& ByteBuffer::get(char* dst, int dstLength,int offset, int length)
		{
		    check_bounds(offset, length, dstLength);
		    if (length > remaining())
		        throw  BufferUnderflowException();
		    arraycopy(hb, ix(position()), dst, offset, length);
		    position(position() + length);
		    return *this;
		}


		inline
		ByteBuffer& ByteBuffer::get(string& dst,int offset, int length)
		{
		    dst.resize(length);
		    check_bounds(offset, length, length);
		    if (length > remaining())
		        throw  BufferUnderflowException();
		    //arraycopy(hb, ix(position()), dst, offset, length);
		    dst.assign(hb+ix(position()), length);
		    position(position() + length);
		    return *this;
		}


		inline
		ByteBuffer& ByteBuffer::get(char* dst,int dstLength, int length)
		{
		    return get(dst, dstLength, 0, length);
		}


		inline
		ByteBuffer& ByteBuffer::get(string& dst, int length)
		{
		    return get(dst, 0, length);
		}


		inline
		ByteBuffer&  ByteBuffer::get_asciiz(char* dst,int dstLength)
		{
		    int i = 0;
		    for(i = 0; i < dstLength; i++)
		    {
		        dst[i]= get();
		        if(dst[i] == 0)
		        {
		            break;
		        }
		    }

		    if((i == (dstLength-1)) && (dst[i] !=0))
		        throw  IndexOutOfBoundsException();

		    return * this;
		}


		inline
		ByteBuffer& ByteBuffer::put(char x)
		{
		    hb[ix(next_put_index())] = x;
		    return *this;
		}

		inline
		ByteBuffer& ByteBuffer::put(int i, char x)
		{
		    hb[ix(check_index(i))] = x;
		    return *this;
		}

		inline
		ByteBuffer& ByteBuffer::put(const char* src, int srcLength,  int offset, int length)
		{
		    check_bounds(offset, length, srcLength);
		    if (length > remaining())
		        throw  BufferOverflowException();
		    arraycopy(const_cast<char*>(src), offset, hb, ix(position()), length);
		    position(position() + length);
		    return *this;
		}


		inline
		ByteBuffer& ByteBuffer::put(const char* src, int srcLength,   int length)
		{
		    return put(src,srcLength , 0, length);
		}

		inline
		ByteBuffer& ByteBuffer::put_asciiz(const byte* src, int srcLength)
		{
		    int length = strlen(src);
		    if(length == 0)
		    {
		        put(0);
		    }
		    else
		    {
		        put(src, srcLength, 0, length);
		        put(0);
		    }

		    return * this;
		}


		inline
		ByteBuffer& ByteBuffer::put(const string& src,  int offset)
		{
		    int length = src.length();
		    check_bounds(offset, length, length);
		    if (length > remaining())
		        throw  BufferOverflowException();
		    arraycopy(const_cast<char*>(src.data()), offset, hb, ix(position()), src.length());
		    position(position() + length);
		    return *this;
		}

		inline
		ByteBuffer& ByteBuffer::put(const string& src)
		{
		    return put(src,  0);
		}

		inline
		ByteBuffer&  ByteBuffer::put(ByteBuffer& src)
		{
		    if (&src == this)
		        throw  IllegalArgumentException();
		    ByteBuffer& sb = src;
		    int n = sb.remaining();
		    if (n > remaining())
		        throw  BufferOverflowException();
		    arraycopy(sb.hb, sb.ix(sb.position()),
		              hb, ix(position()), n);
		    sb.position(sb.position() + n);
		    position(position() + n);

		    return *this;
		}

		inline
		ByteBuffer& ByteBuffer::compact()
		{
		    if( ix(position()) != ix(0))
		    {
		        arraycopy(hb, ix(position()), hb, ix(0), remaining());
		    }
		    position(remaining());
		    limit(capacity());
		    return *this;
		}

		inline
		char ByteBuffer::_get(int i)
		{
		    return hb[i];
		}

		inline
		void ByteBuffer::_put(int i, char b)
		{
		    hb[i] = b;
		}

		// short

		inline
		short ByteBuffer::get_short()
		{
		    return Bits::get_short(*this, ix(next_get_index(2)), _big_endian);
		}

		inline
		short ByteBuffer::get_short(int i)
		{
		    return Bits::get_short(*this, ix(check_index(i, 2)), _big_endian);
		}


		inline
		ByteBuffer& ByteBuffer::put_short(short x)
		{
		    Bits::put_short(*this, ix(next_put_index(2)), x, _big_endian);
		    return *this;
		}

		inline
		ByteBuffer& ByteBuffer::put_short(int i, short x)
		{
		    Bits::put_short(*this, ix(check_index(i, 2)), x, _big_endian);
		    return *this;
		}

		// int


		inline
		int ByteBuffer::get_int()
		{
		    return Bits::get_int(*this, ix(next_get_index(4)), _big_endian);
		}

		inline
		int ByteBuffer::get_int(int i)
		{
		    return Bits::get_int(*this, ix(check_index(i, 4)), _big_endian);
		}


		inline
		ByteBuffer& ByteBuffer::put_int(int x)
		{
		    Bits::put_int(*this, ix(next_put_index(4)), x, _big_endian);
		    return *this;
		}

		inline
		ByteBuffer& ByteBuffer::put_int(int i, int x)
		{
		    Bits::put_int(*this, ix(check_index(i, 4)), x, _big_endian);
		    return *this;
		}

		//long (int 64)
		inline
		int64_t ByteBuffer::get_long()
		{
		    return Bits::get_long(*this, ix(next_get_index(8)), _big_endian);
		}

		inline
		int64_t ByteBuffer::get_long(int i)
		{
		    return Bits::get_long(*this, ix(check_index(i, 8)), _big_endian);
		}


		inline
		ByteBuffer& ByteBuffer::put_long(int64_t x)
		{
		    Bits::put_long(*this, ix(next_put_index(8)), x, _big_endian);
		    return *this;
		}

		inline
		ByteBuffer& ByteBuffer::put_long(int i, int64_t x)
		{
		    Bits::put_long(*this, ix(check_index(i, 8)), x, _big_endian);
		    return *this;
		}

		// float
		inline
		float ByteBuffer::get_float()
		{
		    return Bits::get_float(*this, ix(next_get_index(4)), _big_endian);
		}

		inline
		float ByteBuffer::get_float(int i)
		{
		    return Bits::get_float(*this, ix(check_index(i, 4)), _big_endian);
		}


		inline
		ByteBuffer& ByteBuffer::put_float(float x)
		{
		    Bits::put_float(*this, ix(next_put_index(4)), x, _big_endian);
		    return *this;
		}

		inline
		ByteBuffer& ByteBuffer::put_float(int i, float x)
		{
		    Bits::put_float(*this, ix(check_index(i, 4)), x, _big_endian);
		    return *this;
		}

		static inline
		std::ostream& operator<<(std::ostream& out, const ByteBuffer& byte_buffer)
		{
			byte_buffer.dump(out);
			return out;
		}		
	}
}
#endif

