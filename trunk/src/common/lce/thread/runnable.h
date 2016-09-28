#ifndef __RUNNABLE_H__
#define __RUNNABLE_H__

namespace lce
{
	namespace thread
	{
		class Runnable
		{
		public:

		    /**
		     * Runnables should never throw in their destructors
		     */
		    virtual ~Runnable() {}

		    /**
		     * Task to be performed in another thread of execution
		     */
		    virtual void run() = 0;

		};		
	}
}


#endif
