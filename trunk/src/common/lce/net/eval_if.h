#ifndef _LCE_EVAL_IF_HPP_
#define _LCE_EVAL_IF_HPP_

#include "net/if.h"

namespace lce
{
	namespace net
	{
		template<
		      typename C
		    , typename F1
		    , typename F2
		    >
		struct eval_if
		{
		    typedef typename if_<C,F1,F2>::type f_;
		    typedef typename f_::type type;

		};


		template<
		      bool C
		    , typename F1
		    , typename F2
		    >
		struct eval_if_c
		{
		    typedef typename if_c<C,F1,F2>::type f_;
		    typedef typename f_::type type;

		};	
	}
}
#endif 

