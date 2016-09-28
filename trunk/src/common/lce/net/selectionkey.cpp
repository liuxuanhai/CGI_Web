#include "net/selectablechannel.h"
#include "net/selectionkey.h"
#include "net/selector.h"

namespace lce
{
	namespace net
	{
		SelectionKey::SelectionKey()
		        :_valid(false),
		        _channel(NULL),
		        _selector(NULL),
		        _attachment(NULL),
	                _index(-1),
	                _interest_ops(0),
	                _ready_ops(0)
		{
		}

		SelectionKey::SelectionKey(SelectableChannel& ch, Selector& sel)
		        :_valid(true),
		        _channel(&ch),
		        _selector(&sel),
		        _attachment(NULL),
	                _index(-1),
	                _interest_ops(0),
	                _ready_ops(0)	        
		{
		}

		Attachment* SelectionKey::attach(Attachment* ob)
		{
		    Attachment* a = _attachment;
		    _attachment = ob;
		    return a;
		}

		SelectionKey& SelectionKey::interest_ops(int ops)
		{
		    ensure_valid();
		    return nio_interest_ops(ops);
		}

		SelectableChannel* SelectionKey::channel()
		{
		    return (SelectableChannel *)_channel;
		}

		void SelectionKey::ensure_valid()
		{
		    if (!is_valid())
		        ;//throw new CancelledKeyException();
		}


		SelectionKey& SelectionKey::nio_interest_ops(int ops)
		{
		    if ((ops & ~ channel()->valid_ops()) != 0)
		        ;//throw  IllegalArgumentException;

		    channel()->translate_and_set_interest_ops(ops, *this);
		    _interest_ops = ops;
		    return *this;
		}
	}
}

