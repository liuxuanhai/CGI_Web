#ifndef _FAST_MUTEX_HPP_
#define _FAST_MUTEX_HPP_

#include "thread/lockable.h"
#include "thread/noncopyable.h"


namespace lce
{
    namespace thread
    {
        class FastLock;

        class  FastMutex : public Lockable, private NonCopyable 
        {
          
          FastLock* _lock;

        public:
        
          //! Create a FastMutex
          FastMutex();
        
          //! Destroy a FastMutex
          virtual ~FastMutex();
        
          /**
           * Acquire exclusive access to the mutex. The calling thread will block until the 
           * lock can be acquired. No safety or state checks are performed.
           * 
           * @pre The calling thread should <i>not</i> have previously acquired this lock.
           *      Deadlock will result if the same thread attempts to acquire the mutex more 
           *      than once. 
           *
           * @post The calling thread obtains the lock successfully if no exception is thrown.
           * @exception Interrupted_Exception never thrown
           */
          virtual void acquire();
        
          /**
           * Release exclusive access. No safety or state checks are performed.
           * 
           * @pre the caller should have previously acquired this lock
           */
          virtual void release();
        
          /**
           * Try to acquire exclusive access to the mutex. The calling thread will block until the 
           * lock can be acquired. No safety or state checks are performed.
           * 
           * @pre The calling thread should <i>not</i> have previously acquired this lock.
           *      Deadlock will result if the same thread attempts to acquire the mutex more 
           *      than once. 
           *
           * @param timeout unused
           * @return 
           * - <em>true</em> if the lock was acquired
           * - <em>false</em> if the lock was acquired
           *
           * @post The calling thread obtains the lock successfully if no exception is thrown.
           * @exception Interrupted_Exception never thrown
           */
          virtual bool try_acquire(unsigned long timeout);
        
        }; /* FastMutex */
    
    }
}
  


#endif 
