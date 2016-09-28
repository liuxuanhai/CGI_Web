#include "fast_mutex.h"
#include "fast_lock.h"

namespace lce
{
	namespace thread
	{
		FastMutex::FastMutex() : _lock(new FastLock) { }

		FastMutex::~FastMutex() 
		{
		    delete _lock; 
		}


		void FastMutex::acquire() 
		{
		    _lock->acquire();
		}

		bool FastMutex::try_acquire(unsigned long timeout) 
		{
		    return _lock->try_acquire(timeout);
		}

		void FastMutex::release() 
		 {
		    _lock->release();
		}

	}
}


