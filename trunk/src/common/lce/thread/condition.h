#ifndef __CONDITION_H__
#define __CONDITION_H__

#include "thread/lockable.h"
#include "thread/noncopyable.h"
#include "thread/waitable.h"


namespace lce
{
    namespace thread
    {
        class FifoConditionImpl;

        class  Condition
                    : public Waitable,
                    private NonCopyable
        {

            FifoConditionImpl* _impl;

        public:

            /**
             * Create a Condition associated with the given Lockable object.
             *
             * @param l Lockable object to associate with this Condition object.
             */
            Condition(Lockable& l);

            //! Destroy Condition object
            virtual ~Condition();

            /**
             * Wake <em>one</em> thread waiting on this Condition.
             *
             * The associated Lockable need not have been acquire when this function is
             * invoked.
             *
             * @post a waiting thread, if any exists, will be awakened.
             */
            void signal();

            /**
             * Wake <em>all</em> threads wait()ing on this Condition.
             *
             * The associated Lockable need not have been acquire when this function is
             * invoked.
             *
             * @post all wait()ing threads, if any exist, will be awakened.
             */
            void broadcast();

            /**
             * Wait for this Condition, blocking the calling thread until a signal or broadcast
             * is received.
             *
             * This operation atomically releases the associated Lockable and blocks the calling thread.
             *
             * @exception Interrupted_Exception thrown when the calling thread is interrupted.
             *            A thread may be interrupted at any time, prematurely ending any wait.
             *
             * @pre The thread calling this method must have first acquired the associated
             *      Lockable object.
             *
             * @post A thread that has resumed execution without exception (because of a signal(),
             *       broadcast() or exception) will have acquire()d the associated Lockable object
             *       before returning from a wait().
             *
             * @see Waitable::wait()
             */
            virtual void wait();

            /**
             * Wait for this Condition, blocking the calling thread until a signal or broadcast
             * is received.
             *
             * This operation atomically releases the associated Lockable and blocks the calling thread.
             *
             * @param timeout maximum amount of time (milliseconds) this method could block
             *
             * @return
             *   - <em>true</em> if the Condition receives a signal or broadcast before
             *                   <i>timeout</i> milliseconds elapse.
             *   - <em>false</em> otherwise.
             *
             * @exception Interrupted_Exception thrown when the calling thread is interrupted.
             *            A thread may be interrupted at any time, prematurely ending any wait.
             *
             * @pre The thread calling this method must have first acquired the associated
             *      Lockable object.
             *
             * @post A thread that has resumed execution without exception (because of a signal(),
             *       broadcast() or exception) will have acquire()d the associated Lockable object
             *       before returning from a wait().
             *
             * @see Waitable::wait(unsigned long timeout)
             */
            virtual bool wait(unsigned long timeout);
        };
    }
}



#endif // __ZTCONDITION_H__
