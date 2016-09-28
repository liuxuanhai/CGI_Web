#ifndef __PRIORITY_H__
#define __PRIORITY_H__

namespace lce
{
	namespace thread
	{
		typedef enum 
		{
		    Low,
		    Medium = Low + 1,
		    High   = Low + 2
		} Priority;
	}
}


#endif
