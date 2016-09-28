#ifndef __FASTLOCK_H__
#define __FASTLOCK_H__

#include <pthread.h>
#include <assert.h>

#include "thread/exceptions.h"
#include "thread/noncopyable.h"

namespace lce
{
    namespace thread
    {
        class FastLock : private NonCopyable 
        {

            pthread_mutex_t _mtx;

        public:

            /**
             * Create a new FastLock. No safety or state checks are performed.
             *
             * @exception Initialization_Exception - not thrown
             */
            inline FastLock() {

                if(pthread_mutex_init(&_mtx, 0) != 0)
                    throw Initialization_Exception();

            }

            /**
             * Destroy a FastLock. No safety or state checks are performed.
             */
            inline ~FastLock() {

                if(pthread_mutex_destroy(&_mtx) != 0) {
                    //assert(0);
                }

            }

            /**
             * Acquire an exclusive lock. No safety or state checks are performed.
             *
             * @exception Synchronization_Exception - not thrown
             */
            inline void acquire() {

                if(pthread_mutex_lock(&_mtx) != 0)
                    throw Synchronization_Exception();

            }

            /**
             * Try to acquire an exclusive lock. No safety or state checks are performed.
             * This function returns immediately regardless of the value of the timeout
             *
             * @param timeout Unused
             * @return bool
             * @exception Synchronization_Exception - not thrown
             */
            inline bool try_acquire(unsigned long timeout=0) {

                return (pthread_mutex_trylock(&_mtx) == 0);

            }

            /**
             * Release an exclusive lock. No safety or state checks are performed.
             * The caller should have already acquired the lock, and release it
             * only once.
             *
             * @exception Synchronization_Exception - not thrown
             */
            inline void release() {

                if(pthread_mutex_unlock(&_mtx) != 0)
                    throw Synchronization_Exception();

            }
        }; /* FastLock */
    }
}




#endif
