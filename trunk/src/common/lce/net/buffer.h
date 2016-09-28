#ifndef _LCE_BUFFER_H_
#define _LCE_BUFFER_H_

#include "net/ioexception.h"

namespace lce
{
	namespace net
	{
		class Buffer
		{
		    // Invariants: mark <= position <= limit <= capacity
		private:
		    int _mark;// = -1;
		    int _position;// = 0;
		    int _limit;
		    int _capacity;

		    // Creates a new buffer with the given mark, position, limit, and capacity,
		    // after checking invariants.
		public:

		    Buffer(int mark, int pos, int lim, int cap)
		    {
		        // package-private
		        if (cap < 0)
		            throw  IllegalArgumentException();
		        this->_capacity = cap;
		        limit(lim);
		        position(pos);
		        if (mark > 0)
		        {
		            if (mark > pos)
		                throw  IllegalArgumentException();
		            this->_mark = mark;
		        }
		    }

		    int mark_value(){return _mark;}
		    int position() const{return _position;}
		    int limit() const{return _limit;}
		    int capacity(){ return _capacity;}
		    
		    Buffer& position(int newPosition)
		    {
		        if ((newPosition > _limit) || (newPosition < 0))
		            throw  IllegalArgumentException();
		        _position = newPosition;
		        if (_mark > _position)
		            _mark = -1;
		        return *this;
		    }
		    
		    Buffer& limit(int newLimit)
		    {
		        if ((newLimit > _capacity) || (newLimit < 0))
		            throw  IllegalArgumentException();
		        _limit = newLimit;
		        if (_position > _limit)
		            _position = _limit;
		        if (_mark > _limit)
		            _mark = -1;
		        return *this;
		    }

		    Buffer& mark()
		    {
		        _mark = _position;
		        return *this;
		    }

		    Buffer& reset()
		    {
		        int m = _mark;
		        if (m < 0)
		            throw  InvalidMarkException();
		        _position = m;
		        return *this;
		    }

		    Buffer& clear()
		    {
		        _position = 0;
		        _limit = _capacity;
		        _mark = -1;
		        return *this;
		    }

		    //ready to deal with data, urauclly used after io_read return 
	 	    Buffer& flip()
		    {
		        _limit = _position;
		        _position = 0;
		        _mark = -1;
		        return *this;
		    }

		    Buffer& rewind()
		    {
		        _position = 0;
		        _mark = -1;
		        return *this;
		    }

		    int remaining() const
		    {
		        return _limit - _position;
		    }

		    bool has_remaining()
		    {
		        return _position < _limit;
		    }

		    int next_get_index()
		    {
		        if (_position >= _limit)
		            throw  BufferUnderflowException();
		        return _position++;
		    }

		    int next_get_index(int nb)
		    {
		        if (_position + nb > _limit)
		            throw  BufferUnderflowException();
		        int p = _position;
		        _position += nb;
		        return p;
		    }

		    int next_put_index()
		    {
		        if (_position >= _limit)
		            throw  BufferOverflowException();
		        return _position++;
		    }

		    int next_put_index(int nb)
		    {
		        if (_position + nb > _limit)
		            throw  BufferOverflowException();
		        int p = _position;
		        _position += nb;
		        return p;
		    }

		    int check_index(int i) const
		    {
		        if ((i < 0) || (i >= _limit))
		            throw  IndexOutOfBoundsException();
		        return i;
		    }

		    int check_index(int i, int nb)
		    {
		        if ((i < 0) || (i + nb > _limit))
		            throw  IndexOutOfBoundsException();
		        return i;
		    }

		    static void check_bounds(int off, int len, int size)
		    {
		        // package-private
		        if ((off | len | (off + len) | (size - (off + len))) < 0)
		            throw  IndexOutOfBoundsException();
		    }
		};

	}
}
#endif

