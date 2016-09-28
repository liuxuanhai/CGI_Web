#ifndef _LCE_SELECTORFACTORY_H_
#define _LCE_SELECTORFACTORY_H_

namespace lce
{
	namespace net
	{
		class Selector;
		class PollSelector;
		class EPollSelector;
		class SelectorFactory;
		class SelectorFactory
		{
		private:
		    static SelectorFactory* _factory;
		    SelectorFactory()
		    {
		    }
		public:
		    static SelectorFactory& factory()
		    {
		        if (_factory == NULL)
		        {
		            _factory = new SelectorFactory();
		        }
		        return *_factory;
		    }
		    Selector* open_selector(const char* selector_name);
			Selector* open_selector();
		};	
	}
}
#endif

