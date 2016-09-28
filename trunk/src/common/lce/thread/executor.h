#ifndef _EXECUTOR_H_
#define _EXECUTOR_H_

namespace lce
{
	namespace thread
	{
		template<typename Task>
		class Executor
		{
		public:
			virtual int execute(const Task& task) = 0;

		};		
	}
}

#endif 
