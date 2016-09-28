#ifndef _LCE_CONNECTION_EVENT_LISTENER_H_
#define _LCE_CONNECTION_EVENT_LISTENER_H_

#include <set>

using namespace std;

#include "net/connectionevent.h"

namespace lce
{
	namespace net
	{
		/**
		 * A listener for connection events.
		 */
		class ConnectionEventListener
		{
		    /**
		     * Processes a connection event, encoded in the event object.
		     *
		     * @param e a connection event
		     */
		public:
		    ConnectionEventListener(){}
		    virtual ~ConnectionEventListener(){}
		    virtual void connection_event(ConnectionEvent& e) = 0;
		};

		typedef	set<ConnectionEventListener*> ConnectionEventListernerSet;	
	}
	
}
#endif
