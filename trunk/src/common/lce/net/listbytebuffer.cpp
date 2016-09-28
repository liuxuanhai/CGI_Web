#include "net/listbytebuffer.h"

namespace lce
{
	namespace net
	{
		ListByteBuffer::ListByteBuffer():
			_memory_allocator( NULL ), _slice( false ), _big_endian(true)
		{
			_native_byteorder = (Bits::byte_order().name== ByteOrder::big_endian.name);
		}

		ListByteBuffer::ListByteBuffer( FixedSizeAllocator* allocator ):
			_memory_allocator( allocator ), _slice( false ), _big_endian(true)
		{
			_native_byteorder = (Bits::byte_order().name== ByteOrder::big_endian.name);
		}

		ListByteBuffer::~ListByteBuffer()
		{
			if( NULL != _memory_allocator && !_slice )
			{
				while( NULL != _head )
				{
					pop_byte_buffer_node();
				}
			}
		}


		int ListByteBuffer::push_byte_buffer_node()
		{
			int memory_size = _memory_allocator->size();
			assert( memory_size > (int)( sizeof( Node ) ) );
			char* memory = (char*)(_memory_allocator->allocate());

			if( NULL == memory )
			{
				return -1;
			}

			char* hb = memory + sizeof( Node );
			INode* node = new( memory )Node( hb, memory_size - ( sizeof( Node ) ) );
			
			if( _head == NULL && _tail == NULL )
			{
				_head = node;
				_tail = node;
				
				_position = Position( this, _head );
				_mark = Position( this, _head );
			}
			else
			{
				assert( _head != NULL );
				assert( _head != NULL );
				(dynamic_cast<Node*>(_tail))->_next = dynamic_cast<Node*>(node);
				_tail = node;
			}

			return 0;
		}

		void ListByteBuffer::pop_byte_buffer_node()
		{
			INode* node = ((Node*)(_head))->_next;
			_memory_allocator->deallocate( (char*)(_head) );
			_head = node;
			if( _head == NULL )
			{
				_tail = NULL;
				
				_position = Position( this, NULL );
				_mark = Position( this, NULL );
			}
		}

		ListByteBuffer& ListByteBuffer::cat( ListByteBuffer& src )
		{
			if( (INode*)(-1) == src._limit.node() )
				throw BufferOverflowException();
			
			assert( _memory_allocator == src._memory_allocator );
			if( NULL == src._position.node() || src.remaining() <= 0 )
				return *this;

			if( src._position.node() == src._limit.node() )
				return put( src );

			assert( _position.node() == _tail );

			INode* pre_tail = _tail;
			
			if( push_byte_buffer_node() < 0 )
				throw BufferOverflowException();
			
			if( NULL == pre_tail ) // && NULL == _head
			{
				_head = src._position.node();
				// _tail = _head;
				_position = Position( this, _head, src._position.offset() );
				_mark = _position;
			}
			else
			{
				(dynamic_cast<Node*>(_position.node()))->_next = dynamic_cast<Node*>(src._position.node());
				_position.node()->capacity( _position.offset() );

				int src_head_len = src._position.node()->capacity() - src._position.offset();
				memmove( src._position.node()->buffer(),
					src._position.node()->buffer() + src._position.offset(),
					src_head_len );
				src._position.node()->capacity( src_head_len );
			}

			INode* node = _position.node();
			while( node->next() != src._limit.node() )
			{
				node = node->next();
			}
			
			(dynamic_cast<Node*>(node))->_next = dynamic_cast<Node*>(_tail);

			memcpy( _tail->buffer(), src._limit.node()->buffer(), src._limit.offset() );

			_position = Position( this, _tail, src._limit.offset() );

			if( _limit < _position )
				_limit = _position;

			src._position = src._limit;
			src._head = src._position.node();
			
			return *this;
		}	
	}
}



