#ifndef _LCE_LISTBUFFER_H_
#define _LCE_LISTBUFFER_H_

#include <stdint.h>
#include <limits.h>
#include <assert.h>
#include "net/ioexception.h"

namespace lce
{
	namespace net
	{
		class ListBuffer
		{
		public:
			class INode
			{
			friend class ListBuffer;
			public:
				virtual char* buffer() = 0;
				virtual int capacity() = 0;
				virtual void capacity( int cap ) = 0;
				virtual INode* next()  = 0;
			protected:
				INode(){}
				virtual ~INode(){}
			};

			class Position
			{
			friend class ListBuffer;
			private:
				INode* _node;
				int _offset;
				ListBuffer* _buffer;

			public:
				Position( ListBuffer* buffer, INode* node = NULL, int offset = 0 ):
					_node( node ), 
					_offset( offset ),
					_buffer( buffer )
				{}
			
				Position( const Position& p )
				{
					_buffer = p._buffer;
					_node = p._node;
					_offset = p._offset;
				}
			public:
				INode* node()
				{
					return _node;
				}

				int offset()
				{
					return _offset;
				}

				Position& operator= ( const Position& p )
				{
					if( this == &p )
						return *this;
					_node = p._node;
					_offset = p._offset;
					_buffer = p._buffer;
					return *this;
				}
				
				bool operator== ( const Position& p ) const
				{
					if( _node == p._node && 
						_offset == p._offset &&
						_buffer == p._buffer )
						return true;
					return false;
				}

				bool operator!= ( const Position& p ) const
				{
					if( *this == p )
						return false;
					return true;
				}
				
				bool operator> ( const Position& p ) const
				{
					if( _buffer != p._buffer )
						return false;
					if( _node == p._node )
					{
						return _offset > p._offset;
					}

					if( (INode*)(-1) == p._node )
						return false;

					INode* node = p._node;
					while( NULL != node )
					{
						if( node == _node )
							return true;
						node = node->next();
					}

					return false;
				}
				
				bool operator>= ( const Position& p ) const
				{
					if( *this > p || *this == p )
						return true;
					else
						return false;
				}

				bool operator< ( const Position& p ) const
				{
					if( _buffer != p._buffer )
						return false;
					if( _node == p._node )
					{
						return _offset < p._offset;
					}

					if( (INode*)(-1) == _node )
						return false;

					INode* node = _node;
					while( NULL != node )
					{
						if( node == p._node )
							return true;
						node = node->next();
					}

					return false;
				}

				bool operator<= ( const Position& p ) const
				{
					if( *this < p || *this == p )
						return true;
					else
						return false;
				}

				Position operator+ ( int offset_delta ) const
				{
					if( offset_delta < 0 )
						throw  IllegalArgumentException();
					if( (INode*)(-1) == _node || NULL == _node )
						throw  BufferUnderflowException();
					
					int offset = _offset + offset_delta;
					INode* node = _node;
					while( offset >= node->capacity() )
					{
						if( NULL == node->next() )
						{
							assert( node == _buffer->_tail );
							if( _buffer->push_byte_buffer_node() < 0 )
								throw  BufferUnderflowException();
						}
						assert( NULL != node->next() );
						offset -= _node->capacity();
						node = node->next();
					}
					
					Position tmp( _buffer, node, offset );
					
					return tmp;
				}

				Position &operator+= ( int offset_delta )
				{
					*this = *this + offset_delta;
					return *this;
				}

				int operator- ( const Position& p ) const
				{
					if( *this == p )
						return 0;
					if( (INode*)(-1) == _node )
						return INT_MAX;
					if( (INode*)(-1) == p._node )
						return INT_MIN;

					if( NULL == p._node || NULL == _node )
						throw  BufferUnderflowException();

					if( *this <= p )
					{
						INode* node = _node;
						int result = -_offset;// - node->capacity();
						while( node != p._node )
						{
							result += node->capacity();
							node = node->next();
						}				
						result += ( p._offset );
						return -result;
					}
					if( *this >= p )
					{
						INode* node = p._node;
						int result = -( p._offset );// - node->capacity();
						while( node != _node )
						{
							result += node->capacity();
							node = node->next();
						}
						result += ( _offset );
						return result;
					}

					return 0;
				}

				Position operator++ ( int )
				{
					Position tmp( _buffer, _node, _offset );
					*this = *this + 1;
					return tmp;
				}

				Position operator++ ()
				{
					*this = *this + 1;
					return *this;
				}
			};
			
		    // Invariants: mark <= position <= limit <= capacity
		protected:
			INode* _head;
			INode* _tail;

		    Position _mark;// = -1;
		    Position _position;// = 0;
		    Position _limit;
		    Position _capacity;

		    // Creates a new buffer with the given mark, position, limit, and capacity,
		    // after checking invariants.
		public:

		    ListBuffer() : 
				_head( NULL ),
				_tail( NULL ),
				_mark( this ),
				_position( this ),
				_limit( this, (INode*)(-1) ), 
				_capacity( this, (INode*)(-1) )
		    {
		    	
		    }

			
			virtual ~ListBuffer(){}
		    
		    Position& position()
		    {
		    	if( NULL == _position._node )
				{
					assert( NULL == _head );
					assert( NULL == _tail );
					if( push_byte_buffer_node() < 0 )
						throw BufferUnderflowException();
				}
		        return _position;
		    }

		    ListBuffer& position(Position new_position)
		    {
		        if ( ( this != new_position._buffer ) || 
					( NULL == new_position._node ) || 
					( new_position > _limit ) )
		            throw  IllegalArgumentException();
		        _position = new_position;
		        if ( _mark > _position )
		            _mark = _position; // pay attention
		        return *this;
		    }

		    Position limit() const
		    {
		        return _limit;
		    }

		    Position capacity()
		    {
		        return _capacity;
		    }        

		    ListBuffer& limit(Position new_limit)
		    {
		        if ( ( this != new_limit._buffer ) ||
					( NULL == new_limit._node ) ||
					( new_limit > _capacity ) )
		            throw  IllegalArgumentException();
		        _limit = new_limit;
		        if ( _position > _limit )
		            _position = _limit;
		        if ( _mark > _limit )
		            _mark = _position; // pay attention
		        return *this;
		    }

		    
			
		    ListBuffer& clear()
		    {
		        _position = Position( this, _head );
		        _limit = _capacity;
		        _mark = _position; // pay attention
		        return *this;
		    }
			
			//state op:  init_state--->(read_io,flip,deal,compact)--->(read_io,flip,deal,compact)---> ......
		    //generally， used to deal data, after io_read, match compact
		    ListBuffer& flip()
		    {
		        _limit = _position;
		        reset();
		        return *this;
		    }

		    //generally, used to continue io_read, after deal data, match flip
			ListBuffer& compact()
		    {
				while( _position._node != _head )
				{
					pop_byte_buffer_node();
				}
				assert( NULL != _head );
				
				mark();
				_position = _limit;
				_limit = _capacity;
		        return *this;
		    }
			
		    ListBuffer& rewind()
		    {
		        _position = _mark;
		        return *this;
		    }

		    int remaining()
		    {
		        return _limit - _position;
		    }

		    bool has_remaining()
		    {
		        return _position < _limit;
		    }

			bool has_data()
			{
				return _position > _mark;
			}

		    Position next_get_position()
		    {
		        if (_position >= _limit)
		            throw  BufferUnderflowException();
		        return _position++;
		    }

			
			Position next_get_position(int nb)
			{
		        if ( _position + nb > _limit )
		            throw  BufferUnderflowException();
				
				Position p = _position;
				_position += nb;
				return p;
			}

		    Position next_put_position()
		    {
		    	if( NULL == _position._node )
				{
					assert( NULL == _head );
					assert( NULL == _tail );
					if( push_byte_buffer_node() < 0 )
						throw  BufferUnderflowException();
				}
				assert( NULL != _position._node );
		        return _position++;
		    }

		    Position next_put_position(int nb)
		    {
		    	if( NULL == _position._node )
				{
					assert( NULL == _head );
					assert( NULL == _tail );
					if( push_byte_buffer_node() < 0 )
						throw  BufferUnderflowException();
				}
				assert( NULL != _position._node );
		        Position p = _position;
		        _position += nb;
		        return p;
		    }
			
		    Position check_position(Position i)
		    {
		    	if( i._buffer != this || 
					NULL == i._node )
					throw IllegalArgumentException();

				if( (INode*)(-1) == i._node )
					throw  IndexOutOfBoundsException();

				if( NULL != _head && i >= Position( this, _head ) )
					return i;
				
		        throw  IndexOutOfBoundsException();
		    }

		    Position check_position(Position i, int nb)
		    {
		        check_position( i );
				/*Position test_plus = */   i + nb;

		        return i;
		    }

			int data_length()
			{
				return _position - _mark;
			}

			static void check_bounds(int off, int len, int size)
		    {
		        // package-private
		        if ( ( off | len | (off + len) | (size - (off + len) ) ) < 0 )
		            throw  IndexOutOfBoundsException();
		    }
			
		private:
			ListBuffer& mark()
		    {
		        _mark = _position;
		        return *this;
		    }

		    ListBuffer& reset()
		    {
		        Position m = _mark;
		        if ((_head != NULL || _tail != NULL) && (NULL == m._node || 
					(INode*)(-1) == m._node) )
		            throw  InvalidMarkException();
		        _position = m;
		        return *this;
		    }
			
		protected:
			virtual int push_byte_buffer_node() = 0;
			virtual void pop_byte_buffer_node() = 0;	
		};
	}
}
#endif // #ifndef _GCE_LISTBUFFER_H_

