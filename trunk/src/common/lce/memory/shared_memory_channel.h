#ifndef _SHM_CHANNEL_H_
#define _SHM_CHANNEL_H_

#include <sstream>
#include <string>
#include "memory/shared_memory.h"

using namespace std;

namespace lce
{
	namespace memory
	{
		template<class ElemType, size_t Capacity> 
		class SharedMemoryChannel: public SharedMemory
		{
			typedef ElemType value_type;
			typedef ElemType& reference;
			typedef const ElemType& const_reference;

		protected:
			size_t _head;
			size_t _tail;
			value_type _elements[Capacity];

		public:
			SharedMemoryChannel()
			{
				if(is_new())
				{
					//creator need to init the shm content,  while getter only use it
					_head = 0;
					_tail = 0;
				}
			}

			~SharedMemoryChannel()
			{
			}

			size_t capacity() const
			{
				return Capacity;
			}

			size_t size() const
			{
				return (Capacity - _head + _tail)  % Capacity;
			}

			bool full() 
			{
				return size() == (Capacity -1);
			}

			bool empty()
			{
				return _head == _tail;
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
				return _elements[(Capacity +_tail -1) % Capacity];
			}

			const_reference back() const
			{
				return _elements[(Capacity +_tail -1) % Capacity];
			}

			int push(const_reference value)
			{
				if(size() == (Capacity -1))
				{
					return -1;
				}
				else
				{
					_elements[_tail] = value;
					_tail = (_tail+1) % Capacity;
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
                    _head = (_head + 1) % Capacity;
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

			string dumpinfo()
			{
				stringstream outstream;
				outstream<<"{"
					<<"shmid: "<<get_shmid()
					<<", shmkey: "<<get_shmkey()
					<<", isnew:"<<is_new()
					<<", head:"<<_head
					<<", tail:"<<_tail
					<<", capacity:"<<Capacity;

				return outstream.str();
			}
		};
	}
}

#endif
