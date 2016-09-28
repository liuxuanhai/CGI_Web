#include <string.h>
#include "net/pollselector.h"
#include "net/epollselector.h"
#include "net/selectorfactory.h"

namespace lce
{
	namespace net
	{
		SelectorFactory* SelectorFactory::_factory = NULL;

		Selector* SelectorFactory::open_selector()
		{
		    return new PollSelector();
		}

		Selector* SelectorFactory::open_selector(const char* selector_name)
		{
		    if(strcmp(selector_name,"poll") == 0)
		    {
		        return new PollSelector();
		    }
		    else if(strcmp(selector_name,"epoll") == 0)
		    {
		        return new EPollSelector();
		    }
		    else
		    {
		        return NULL;
		    }
		}	
	}
}

