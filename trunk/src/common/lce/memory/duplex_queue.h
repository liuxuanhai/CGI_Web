#ifndef _LCE_MEMORY_DEPLEX_QUEUE__H_
#define _LCE_MEMORY_DEPLEX_QUEUE__H_

#include "memory/array_circular_queue.h"
#include <assert.h>

namespace lce
{
	namespace memory
	{
		template<typename T, typename U>
		class DuplexQueue
		{
		public:
			DuplexQueue()
			{
			}
			~DuplexQueue()
			{
			}
		public:
			int init(unsigned int size)
			{
				assert(size>0);
				if(_request_queue.init(size+1)<0)
				{
					return -1;
				}
				if(_reply_queue.init(size+1)<0)
				{
					return -1;
				}
				
				return 0;
			}
		
			
		public:
		
			//request queue
			inline int request_size()
			{
				return _request_queue.size();
			}
			inline bool request_full()
			{
				return _request_queue.full();
			}
			inline bool request_empty()
			{
				return _request_queue.empty();
			}
			inline T& get_request()
			{
				return _request_queue.front();
			}
			inline void pop_request()
			{
				T& _t = _request_queue.front();
				_t.~T();
				new(&_t) T();
				_request_queue.pop();
			}
			inline int push_request(const T& request)
			{
				return _request_queue.push(request);
			}
		
		
			//reply queue
			inline int reply_size()
			{
				return _reply_queue.size();
			}
			inline bool reply_full() 
			{
				return _reply_queue.full();
			}
			inline bool reply_empty()
			{
				return _reply_queue.empty();
			}
			inline U& get_reply()
			{
				return _reply_queue.front();
			}
			inline void pop_reply()
			{
				U& _u = _reply_queue.front();
				_u.~U();
				new(&_u) U();
				
				_reply_queue.pop();
			}
			inline int push_reply(const U& reply)
			{
				return _reply_queue.push(reply);
			}
		
		private:
			ArrayCircularQueue<T> _request_queue;
			ArrayCircularQueue<U> _reply_queue;
		};
				
	}
}

#endif



