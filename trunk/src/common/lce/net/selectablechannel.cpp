#include "net/socketaddress.h"
#include "net/selectionkey.h"
#include "net/selector.h"
#include "net/ioexception.h"
#include "net/selectablechannel.h"

namespace lce
{
	namespace net
	{
		SelectableChannel::SelectableChannel()
		        :_key(NULL),
		        _registered(false),
		        _blocking(true),
		        _open(false)
		{
		}

		SelectableChannel::~SelectableChannel()
		{
		    close();
		}
		SelectionKey* SelectableChannel::enregister(Selector& sel, int ops)
		{
		    return enregister(sel, ops, NULL);
		}

		SelectionKey* SelectableChannel::enregister(Selector& sel, int ops, Attachment* att)
		{
		    if (!is_open())
		        return NULL;

		    if ((ops & (~ valid_ops())) != 0)
		        return NULL;

		    if (_blocking)
		        return NULL;

		    if (_registered )
		    {
		        if(_key->selector() == &sel )
		        {
		            _key->interest_ops(ops);
		        }
		        else
		        {
		            deregister();
		            _key = sel.enregister(*this, ops, att);
			   		_registered = (NULL != _key);
		        }
		    }
		    else
		    {
		        _key = sel.enregister(*this, ops, att);
		        _registered = (NULL != _key);
		    }

		    return _key;
		}

		int SelectableChannel::deregister()
		{
		    if(_registered)
		    {
		        _key->selector()->cancel(*_key);
		        _registered = false;
		        _key = NULL;
		    }
		    return 0;
		}

		SelectableChannel& SelectableChannel::configure_blocking(bool block)
		{
		    if (!is_open())
		    {
		        return *this;
		    }

		    if (_blocking == block)
		    {
		        return *this;
		    }

		    if (block && _registered && _key->is_valid())
		    {
		        return *this;
		    }

		    impl_configure_blocking(block);
		    _blocking = block;

		    return *this;
		}

		void SelectableChannel::close()
		{
		    if (_open)
		    {
		        _open = false;
		    }

	        if (!_blocking)
	        {
	            _blocking = true;
	        }
		}	
	}
}


