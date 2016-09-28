#ifndef _LCE_IDENTITY_HPP_
#define _LCE_IDENTITY_HPP_

namespace lce
{
	namespace net
	{
		template<typename T>
		struct identity
		{
		    typedef T type;
		};

		template<typename T>
		struct make_identity
		{
		    typedef identity<T> type;
		};	
	}
	
}

#endif 

