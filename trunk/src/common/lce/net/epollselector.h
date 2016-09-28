#ifndef _LCE_BASE_NET_EPOLLSELECTOR_H_
#define _LCE_BASE_NET_EPOLLSELECTOR_H_

#include <ext/hash_map>
#include "net/selector.h"
#include "thread/runnable.h"
#include "net/selectionkey.h"
#include "net/selectablechannel.h"
#include "net/epollwrapper.h"
#include "net/net.h"

using namespace std;
using namespace __gnu_cxx;

namespace lce
{
	namespace net
	{
		class EPollSelector : public Selector
		{
		private:
		    int _capacity;
		    // The poll fd array
		    EPollArrayWrapper _epoll_wrapper;
		    // The list of SelectableChannels serviced by this Selector
		    SelectionKeyPool _selection_key_pool;    

	        typedef hash_map<int, SelectionKey*> ChannelMap;
		    ChannelMap _channel_array;

		    // The number of valid channels in this Selector's poll array
		    int _selected_channel_num;

		    // File descriptors used for interrupt
		    int _fd0;
		    int _fd1;

		    // Lock for interrupt triggering and clearing
		    bool _interrupt_triggered;
		    bool _selector_open;

		    int   _iterator_pos;
		public:

		    EPollSelector();
		    virtual ~EPollSelector();

		    int  init(int max_channels);
		    void close();
		    bool is_open();

		    int select();
		    int select(long timeout);
		    int select_now();

		    bool has_next();
		    SelectionKey* first();
		    SelectionKey* next();

		    Selector& wakeup();
		    void cancel(SelectionKey& k);
		    void put_event_ops(SelectionKey& sk, int ops) ;

		protected:
		    SelectionKey* enregister(SelectableChannel& ch, int ops, Attachment* attachment);
		    void deregister(SelectionKey& key);

		private:
		    int do_select(long timeout);
		    void close_interrupt();
		    void proc_interrupt();
		    SelectionKey*  selected_key(int pos );
		    int translate_ready_ops(int oldOps);
		    int translate_interest_ops(int ops);

		};

		inline
		bool EPollSelector::is_open()
		{
		    return _selector_open;
		}

		inline
		SelectionKey*  EPollSelector::selected_key(int pos)
		{
		    epoll_event* event = _epoll_wrapper.get_revent_ops(pos);

	        ChannelMap::const_iterator it = _channel_array.find(event->data.fd);
	        if (it == _channel_array.end())
	        {
	            return NULL;
	        }
	        
		    SelectionKey* sk = it->second;
	        assert(sk != NULL);
	        
		    int returned_ops = event->events;

		    sk->channel()->translate_and_set_ready_ops(returned_ops, *sk);
		    if ((sk->nio_ready_ops() & sk->nio_interest_ops()) != 0)
		    {
		        return sk;
		    }
		    else
		    {
		        return NULL;
		    }
		}

		inline
		void EPollSelector::proc_interrupt()
		{
		    epoll_event* event = _epoll_wrapper.get_revent_ops(_iterator_pos);

		    if (event->data.fd == _fd0)
		    {
		        // Clear the wakeup pipe
		        Net::drain(_fd0);
		        _interrupt_triggered = false;
		        _iterator_pos++;
		    }

		}
	}
}
#endif

