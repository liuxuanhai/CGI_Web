#ifndef __LOCKABLE_H__
#define __LOCKABLE_H__

#include "thread/exceptions.h"

namespace lce
{
    namespace thread
    {
        class Lockable
        {
        public:

            //! Destroy a Lockable object.
            virtual ~Lockable() {}

            /**
             * Acquire the Lockable object.
             *
             * This method may or may not block the caller for an indefinite amount
             * of time. Those details are defined by specializations of this class.
             *
             * @exception Interrupted_Exception thrown if the calling thread is interrupted before
             *            the operation completes.
             *
             * @post The Lockable is acquired only if no exception was thrown.
             */
            virtual void acquire() = 0;

            /**
             * Attempt to acquire the Lockable object.
             *
             * This method may or may not block the caller for a definite amount
             * of time. Those details are defined by specializations of this class;
             * however, this method includes a timeout value that can be used to
             * limit the maximum amount of time that a specialization <i>could</i> block.
             *
             * @param timeout - maximum amount of time (milliseconds) this method could block
             *
             * @return
             *   - <em>true</em>  if the operation completes and the Lockable is acquired before
             *     the timeout expires.
             *   - <em>false</em> if the operation times out before the Lockable can be acquired.
             *
             * @exception Interrupted_Exception thrown if the calling thread is interrupted before
             *            the operation completes.
             *
             * @post The Lockable is acquired only if no exception was thrown.
             */
            virtual bool try_acquire(unsigned long timeout) = 0;

            /**
             * Release the Lockable object.
             *
             * This method may or may not block the caller for an indefinite amount
             * of time. Those details are defined by specializations of this class.
             *
             * @post The Lockable is released only if no exception was thrown.
             */
            virtual void release() = 0;

        };

    }
}




#endif // __ZTL
