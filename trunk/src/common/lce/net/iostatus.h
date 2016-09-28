#ifndef _LCE_IOSTATUS_H_
#define _LCE_IOSTATUS_H_

#include <stdint.h>

namespace lce
{
	namespace net
	{
		class IOStatus
		{    
		private:
			IOStatus(){}
		public:
			static const int IOEOF = 1025;
			static const int UNAVAILABLE = 1026; // Nothing available (non-blocking)    
			static const int INTERRUPTED = 1027; // System call interrupted    
			static const int UNSUPPORTED = 1028; // Operation not supported    
			static const int UNSUPPORTED_CASE = 1029; // This case not supported
		};		
	}	
}
#endif
