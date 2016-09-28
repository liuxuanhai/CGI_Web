#ifndef __CANCELABLE_H__
#define __CANCELABLE_H__

#include "thread/exceptions.h"

namespace lce
{
    namespace thread
    {
        class Cancelable
        {
        public:

            //! Destroy a Cancelable object.
            virtual ~Cancelable() {}

            /**
             * Canceling a Cancelable object makes a request to disable that object.
             * This entails refusing to take on any new responsibility, and completing
             * its current responsibilities quickly.
             *
             * Canceling an object more than once has no effect.
             *
             * @post The Cancelable object will have permanently transitioned to a
             *       disabled state; it will now refuse to accept new responsibility.
             */
            virtual void cancel() = 0;

            /**
             * Determine if a Cancelable object has been canceled.
             *
             * @return
             *   - <em>true</em> if cancel() was called prior to this function.
             *   - <em>false</em> otherwise.
             */
            virtual bool is_canceled() = 0;

        }; /* Cancelable */       
    }
}

#endif
