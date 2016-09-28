#ifndef _ARRAY_CIRCULAR_QUEUE_
#define _ARRAY_CIRCULAR_QUEUE_

#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>

namespace lce
{
	namespace memory
	{
		template <typename ElementType>
		class ArrayCircularQueue
		{
	    public:
			typedef ElementType               value_type;
			typedef ElementType&              reference;
			typedef const ElementType&        const_reference;

		public:
			ArrayCircularQueue()
				: _elements(NULL)
                , _head(0)
                , _tail(0)
                , _capacity(0)
    		{
    		}
			explicit ArrayCircularQueue(size_t  capacity)
				: _elements(NULL)
                , _head(0)
                , _tail(0)
                , _capacity(capacity)
    		{
    			_elements = new value_type[_capacity];
    		}

			~ArrayCircularQueue()
			{
				delete[]  _elements;
			}

			int init(size_t  capacity)
			{
				_capacity = capacity;
                assert(_capacity > 0 && NULL == _elements);
				_elements = new value_type[capacity];
				if(NULL == _elements)
				{
					return -1;
				}
				
				return 0;
			}
			
        	size_t capacity()
			{
				return _capacity;
			}

			bool full()
			{
				return size() == (_capacity -1);
			}

			bool empty()
			{
				return _head == _tail;
			}

			size_t size() const
			{
				return (_capacity -_head+_tail) %_capacity;
			}

			reference front()
			{
				return _elements[_head];
			}

			const_reference front() const
			{
				return _elements[_head];
			}

			reference back()
			{
				return _elements[(_capacity +_tail -1) % _capacity];
			}

			const_reference back() const
			{
				return _elements[(_capacity +_tail -1) % _capacity];
			}

			int push(const_reference value)
			{
				if(size() == (_capacity -1))
				{
					return -1;
				}
				else
				{
					_elements[_tail] = value;
					_tail = (_tail+1) % _capacity;
					return 0;
				}
			}

			void pop()
            {
                if(empty())
                {
                    return;
                }
                else
                {
                    _head = (_head + 1) % _capacity;
                }
            }
    
            void pop(reference r)
            {
                if(empty())
                {
                    return;
                }
                
                r=front();
                pop();
                return;
            }

			void clear()
			{
				_head = 0;
				_tail = 0;
			}

		private:
			value_type* _elements;
			volatile  uint32_t      _head;
			volatile uint32_t      _tail;
			size_t            _capacity;
		};
	}
}
#endif
