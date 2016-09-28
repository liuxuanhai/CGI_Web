#ifndef _FULL_DUPLEX_QUEUE_H_
#define _FULL_DUPLEX_QUEUE_H_

#include "memory/array_circular_queue.h"
#include <assert.h>

namespace lce
{
	namespace memory
	{
		template<typename RequestElementType, typename ReplyElementType>
		class FullDuplexQueue
		{
		public:
			FullDuplexQueue()
			{
			}
			~FullDuplexQueue()
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

			//commonly,  A as Client, while B as Server
			//especially,  A as MainThread , while B as one WorkerThread

		public:
			// A --> B
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
			inline RequestElementType& get_request()
			{
				return _request_queue.front();
			}
			inline void pop_request()
			{
				RequestElementType& _t = _request_queue.front();
				_t.~RequestElementType();
				new(&_t) RequestElementType();
				_request_queue.pop();
			}
			inline int push_request(const RequestElementType& request)
			{
				return _request_queue.push(request);
			}

			//B ---> A
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
			inline ReplyElementType& get_reply()
			{
				return _reply_queue.front();
			}
			inline void pop_reply()
			{
				ReplyElementType& _u = _reply_queue.front();
				_u.~ReplyElementType();
				new(&_u) ReplyElementType();
				_reply_queue.pop();
			}
			inline int push_reply(const ReplyElementType& reply)
			{
				return _reply_queue.push(reply);
			}

		private:
			ArrayCircularQueue<RequestElementType> _request_queue;
			ArrayCircularQueue<ReplyElementType>   _reply_queue;
		};

	}
}

#endif

