#ifndef __WAITABLE_H__
#define __WAITABLE_H__

#include "thread/exceptions.h"

namespace lce
{
    namespace thread
    {
        class Waitable
        {
        public:

            //! Destroy a Waitable object.
            virtual ~Waitable() {}

            /**
             * Waiting on an object will generally cause the calling thread to be blocked
             * for some indefinite period of time. The thread executing will not proceed
             * any further until the Waitable object releases it unless or an exception
             * is thrown.
             */
            virtual void wait() = 0;

            /**
             * Waiting on an object will generally cause the calling thread to be blocked
             * for some indefinite period of time. The thread executing will not proceed
             * any further until the Waitable object releases it unless or an exception
             * is thrown.
             *
             * @param timeout maximum amount of time, in milliseconds, to spend waiting.
             *
             * @return
             *   - <em>true</em> if the set of tasks being wait for complete before
             *                   <i>timeout</i> milliseconds elapse.
             *   - <em>false</em> othewise.
             */
            virtual bool wait(unsigned long timeout) = 0;


        }; /* Waitable */

    }
}




#endif
