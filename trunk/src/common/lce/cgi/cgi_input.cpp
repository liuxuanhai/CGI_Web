#include "cgi_input.h"

namespace lce
{
	namespace cgi
	{
		bool CCgiInput::Init()
		{
			return m_oEnv.Init();
		}
	}
}


