#ifndef _LCE_BYTEBUFFERLIST_H_
#define _LCE_BYTEBUFFERLIST_H_

#include <stdint.h>
#include <string>
#include <string.h>
#include "memory/fixed_size_allocator.h"
#include "net/byteorder.h"
#include "net/bytebuffer.h"
#include "net/bits.h"
#include "net/listbuffer.h"

using namespace std;
using namespace lce::memory;

namespace lce
{
	namespace net
	{
		typedef char byte;
		class ByteOrder;
		class Bits;
		class ConnectionInfo;

		class ListByteBuffer : public ListBuffer
		{
		friend class ConnectionInfo;

		private:
			union FloatToInt
			{
				float f;
				int i;
			};

		public:
			class Node : public INode
			{
			friend class ListByteBuffer;
			public:
				char* buffer(){ return _buffer; }
				int capacity(){ return _capacity; }
				void capacity( int cap )
				{ 
					if( cap < 0 )
						throw IllegalArgumentException();
					_capacity = cap;
				}
				INode* next(){ return _next; }
				
			private:
				
				char* _buffer;
				int _capacity;
				Node* _next;

				Node( char* buffer, int cap ):
					_buffer( buffer ), _capacity( cap ), _next( NULL )
				{
				}
			};

			ListByteBuffer();
			ListByteBuffer(FixedSizeAllocator* allocator );
			~ListByteBuffer();
			
		    ListByteBuffer& get( char* dst, int dstLength, int length );
		    ListByteBuffer& get( string& dst, int length );


		    ListByteBuffer& put( const char* src, int srcLength, int length );
		    ListByteBuffer& put( const string& src );

			ListByteBuffer& put( ByteBuffer& src );
			ListByteBuffer& put( ListByteBuffer& src );
			ListByteBuffer& cat( ListByteBuffer& src );

			byte _get(Position p) ;

		    void _put(Position p, byte b);

			//byte
		    char get() ;
		    char get( Position p );
		    ListByteBuffer& put( char x );
		    ListByteBuffer& put( Position p, char x );

			// short
		    short get_short();
		    short get_short( Position p );
		    ListByteBuffer& put_short( short x );
		    ListByteBuffer& put_short( Position p, short x );
			
		    // int
		    int get_int();
		    int get_int( Position p );
		    ListByteBuffer& put_int( int x );
		    ListByteBuffer& put_int( Position p, int x );

		    //long (int64)
		    int64_t get_long();
		    int64_t get_long( Position p );
		    ListByteBuffer& put_long( int64_t x );
		    ListByteBuffer& put_long( Position p, int64_t x );
		    
		    // float
		    float get_float();
		    float get_float( Position p );
		    ListByteBuffer& put_float( float x );
		    ListByteBuffer& put_float( Position p, float x );	

		    ByteOrder order();
		    ListByteBuffer& order(ByteOrder bo);

			void slice( ListByteBuffer& lbb );
			void follow_down();

			ssize_t copy_to(ByteBuffer& bb);
			

		protected:
			int push_byte_buffer_node();
			void pop_byte_buffer_node();

		private:

			FixedSizeAllocator* _memory_allocator;
			bool _slice;

			bool _big_endian;
			bool _native_byteorder;
			
		};

		// string
		inline ListByteBuffer& ListByteBuffer::get( char* dst, int dstLength, int length )
		{
			if( length <= 0 )
				return *this;
			
			check_bounds( 0, length, dstLength );
		    if( length > remaining() )
		        throw  BufferUnderflowException();

			Position p = position();
			char* dest = dst;
			while( length > 0 )
			{
				int get_from_node_len = length > p.node()->capacity() - p.offset() ? 
					p.node()->capacity() - p.offset() : length;
				//length or get_from_node_len?
				memmove( dest, p.node()->buffer() + p.offset(), get_from_node_len );
				dest += get_from_node_len;
				length -= get_from_node_len;
				p += get_from_node_len;
			}
			
			position( p );
			
		    return *this;
		}

		inline ListByteBuffer& ListByteBuffer::get( string& dst, int length )
		{
			if( length <= 0 )
				return *this;
			
			check_bounds( 0, length, length );
		    if( length > remaining() )
		        throw  BufferUnderflowException();

			dst.reserve(length);

			Position p = position();
			while( length > 0 )
			{
				int get_from_node_len = length > p.node()->capacity() - p.offset() ? 
					p.node()->capacity() - p.offset() : length;
				//length or get_from_node_len?
				dst.append(p.node()->buffer() + p.offset(), get_from_node_len);
				length -= get_from_node_len;
				p += get_from_node_len;
			}
			
			position( p );
			
		    return *this;
		}

		inline ListByteBuffer& ListByteBuffer::put( const char* src, int srcLength, int length )
		{
			if( length <= 0 )
				return *this;
			
			check_bounds( 0, length, srcLength );
		    if( length > remaining() )
		        throw  BufferOverflowException();

			Position p = position();
			const char* data = src;
			while( length > 0 )
			{
				int put_in_node_len = length > p.node()->capacity() - p.offset() ? 
					p.node()->capacity() - p.offset() : length;
				memmove( p.node()->buffer() + p.offset(), data, put_in_node_len );
				data += put_in_node_len;
				length -= put_in_node_len;
				p += put_in_node_len;
			}
			
		    position( p );
			
		    return *this;
		}

		inline ListByteBuffer& ListByteBuffer::put( const string& src )
		{
			return put( src.c_str(), src.size(), src.size() );
		}

		inline ListByteBuffer& ListByteBuffer::put( ByteBuffer& src )
		{
		    ByteBuffer& sb = src;
		    int n = sb.remaining();
		    put( sb.address() + sb.position(), n, n );
		    sb.position( sb.position() + n );
			return *this;
		}

		inline ListByteBuffer& ListByteBuffer::put( ListByteBuffer& src )
		{
			if( (INode*)(-1) == src._limit.node() )
				throw BufferOverflowException();

			if( NULL == src._position.node() || src.remaining() <= 0 )
				return *this;
			
			INode* node = src._position.node();
			while( 1 )
			{
				if( node == src._limit.node() )
					break;
				
				int offset = src._position.offset();
				int capacity = node->capacity();
				if( capacity > offset )
				{
					put( node->buffer() + offset, capacity - offset, capacity - offset );
					src.position( src.position() + ( capacity - offset ) );
					node = src._position.node();			
				}			
			}
			
			int offset = src._position.offset();
			int capacity = src._limit.offset();
			if( capacity > offset )
			{
				put( node->buffer() + offset, capacity - offset, capacity - offset );
				src.position( src.position() + ( capacity - offset ) );
			}		
			return *this;
		}

		inline void ListByteBuffer::slice( ListByteBuffer& lbb )
		{
			//Position p = this->position();
			lbb.~ListByteBuffer();
			new( &lbb )ListByteBuffer;
			lbb._memory_allocator = _memory_allocator;
			lbb._slice = true;
			
			lbb._head = position().node();
			lbb._tail = _tail;
			
			lbb._position = Position( &lbb, position().node(), position().offset() );
			lbb._mark = lbb._position;

			lbb._limit = Position( &lbb, _limit.node(), _limit.offset() );
		}


		inline
		ByteOrder ListByteBuffer::order()
		{
		    return _big_endian ? ByteOrder::big_endian: ByteOrder::little_endian;
		}

		inline
		ListByteBuffer& ListByteBuffer::order(ByteOrder bo)
		{
		    _big_endian = (bo.name == ByteOrder::big_endian.name);
		    _native_byteorder =
		        (_big_endian == (Bits::byte_order().name == ByteOrder::big_endian.name));
		    return *this;
		}


		inline void ListByteBuffer::follow_down()
		{
			while( _tail->next() != NULL )
			{
				_tail = _tail->next();
			}
		}

		//byte
		inline ListByteBuffer& ListByteBuffer::put(char x)
		{
			Position p = next_put_position();
			( p.node()->buffer() )[p.offset()] = x;
		    return *this;
		}

		inline ListByteBuffer& ListByteBuffer::put( Position p, char x )
		{
			check_position( p );
			( p.node()->buffer() )[p.offset()] = x;
		    return *this;
		}
			

		inline char ListByteBuffer::get()
		{
			Position p = next_get_position();
			return ( p.node()->buffer() )[p.offset()];
		}

		inline char ListByteBuffer::get( Position p )
		{
			check_position( p );
			return ( p.node()->buffer() )[p.offset()];
		}

		// short
		inline short ListByteBuffer::get_short()
		{
			return Bits::get_short(*this, next_get_position(2), _big_endian);
			//return (  static_cast<short>( get() ) << 8 ) | 
				//	( get() & 0xFF );
		}

		inline short ListByteBuffer::get_short( Position p ) 
		{
			return Bits::get_short(*this, check_position(p, 2), _big_endian);
			//return (  static_cast<short>( get( p ) ) << 8 ) | 
					//( get( p + 1 ) & 0xFF );
		}

		inline ListByteBuffer& ListByteBuffer::put_short( short x )
		{
			Bits::put_short(*this, next_put_position(2), x, _big_endian);
			//put( static_cast<char>(x >> 8) );
			//put( static_cast<char>(x) );
			return *this;
		}

		inline ListByteBuffer& ListByteBuffer::put_short( Position p, short x )
		{
			Bits::put_short(*this, check_position(p, 2), x, _big_endian);
			//put( p, static_cast<char>(x >> 8) );
			//put( p + 1, static_cast<char>(x) );
			return *this;
		}

		// int
		inline int ListByteBuffer::get_int()
		{
			return Bits::get_int(*this, next_get_position(4), _big_endian);
			//return (  static_cast<int>( get_short() ) << 16 ) | 
					//( get_short() & 0xFFFF );
		}

		inline int ListByteBuffer::get_int( Position p )
		{
			return Bits::get_int(*this, check_position(p, 4), _big_endian);
			//return (  static_cast<int>( get_short( p ) ) << 16 ) | 
					//( get_short( p + 2 ) & 0xFFFF );
		}

		inline ListByteBuffer& ListByteBuffer::put_int( int x )
		{
			Bits::put_int(*this, next_put_position(4), x, _big_endian);
			//put_short( static_cast<short>(x >> 16) );
			//put_short( static_cast<short>(x) );
			return *this;
		}

		inline ListByteBuffer& ListByteBuffer::put_int( Position p, int x )
		{
			Bits::put_int(*this, check_position(p, 4), x, _big_endian);
			//put_short( p, static_cast<short>(x >> 16) );
			//put_short( p + 2, static_cast<short>(x) );
			return *this;
		}

		//long (int64)
		inline int64_t ListByteBuffer::get_long()
		{
			return Bits::get_long(*this, next_get_position(8), _big_endian);
			//return (  static_cast<int64_t>( get_int() ) << 32 ) | 
					//( get_int() & 0xFFFFFFFF );
		}

		inline int64_t ListByteBuffer::get_long( Position p )
		{
			return Bits::get_long(*this, check_position(p, 8), _big_endian);
			//return (  static_cast<int64_t>( get_int( p ) ) << 32 ) | 
				//	( get_int( p + 4 ) & 0xFFFFFFFF );
		}

		inline ListByteBuffer& ListByteBuffer::put_long( int64_t x )
		{
			Bits::put_long(*this, next_put_position(8), x, _big_endian);
			//put_int( static_cast<int>(x >> 32) );
			//put_int( static_cast<int>(x) );
			return *this;
		}

		inline ListByteBuffer& ListByteBuffer::put_long( Position p, int64_t x )
		{
			Bits::put_long(*this, check_position(p, 8), x, _big_endian);
			//put_int( p, static_cast<int>(x >> 32) );
			//put_int( p + 4, static_cast<int>(x) );
			return *this;
		}

		// float
		inline float ListByteBuffer::get_float()
		{
			FloatToInt fi;
			fi.i = get_int();
			return fi.f;
		}

		inline float ListByteBuffer::get_float( Position p )
		{
			FloatToInt fi;
			fi.i = get_int( p );
			return fi.f;
		}

		inline ListByteBuffer& ListByteBuffer::put_float( float x )
		{
			FloatToInt fi;
			fi.f = x;
			return put_int( fi.i );
		}

		inline ListByteBuffer& ListByteBuffer::put_float( Position p, float x )
		{
			FloatToInt fi;
			fi.f = x;
			return put_int( p, fi.i );
		}

		inline ssize_t ListByteBuffer::copy_to(ByteBuffer& bb)
		{
			int other_pos = bb.position();
			int other_lim = bb.limit();

			int other_rem = (other_pos <= other_lim ? other_lim - other_pos : 0);

			if(other_rem>remaining())
				other_rem = remaining();

			Position p = position();

			int rem = 0;
			int pos = other_pos;
			
			while( other_rem > 0 )
			{
				int get_from_node_len = other_rem > p.node()->capacity() - p.offset() ? 
					p.node()->capacity() - p.offset() : other_rem;


				memcpy(bb.address() + other_pos, p.node()->buffer() + p.offset(), get_from_node_len);
				other_pos += get_from_node_len;
				other_rem -= get_from_node_len;
				rem += get_from_node_len;

				if( p.node() == limit().node())
					break;
				
				p += get_from_node_len;
			}

			try
			{
				bb.limit(pos + rem);
			}
			catch(std::exception&)
			{
				return -1;
			}
			
			return static_cast<ssize_t>(rem);
		}

		inline byte ListByteBuffer::_get(Position p)
		{
			return ( p.node()->buffer() )[p.offset()];
		}

		inline void ListByteBuffer::_put(Position p, byte b)
		{
			( p.node()->buffer() )[p.offset()] = b;
		}
	
	}
	
}
#endif // ifndef

