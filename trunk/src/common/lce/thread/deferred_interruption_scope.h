#ifndef __DEFERREDINTERRUPTIONSCOPE_H__
#define __DEFERREDINTERRUPTIONSCOPE_H__

#include <cassert>
#include "guard.h"
#include "thread.h"


namespace lce
{
	namespace thread
	{
		class DeferredInterruptionScope
		{
		public:

		    template <class LockType>
		    static void createScope(LockHolder<LockType>& l) {

		        l.getLock().interest(Monitor::SIGNALED);

		    }

		    template <class LockType>
		    static void destroyScope(LockHolder<LockType>& l) {

		        l.getLock().interest(Monitor::ANYTHING);
		    }
		};
	}
}


#endif
