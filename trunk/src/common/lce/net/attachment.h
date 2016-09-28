#ifndef _LCE_ATTACHMENT_H_
#define _LCE_ATTACHMENT_H_

#include "net/selectionkey.h"

namespace lce
{
	namespace net
	{
		class Attachment
		{
		public:
		    virtual ~Attachment(){}
		    virtual  int run(SelectionKey& key)=0;

		};	
	}
}
#endif

