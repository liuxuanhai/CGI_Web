#ifndef _PID_FILE_HPP_
#define _PID_FILE_HPP_

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

namespace lce
{
	namespace app
	{
		class Pid
		{
		public:
		    static int  read_pid(const char* fname, pid_t& pid);
		    static int  write_pid(const char* fname);    
		    static bool exists_pid(char* fname);
		    static bool exists_pid(pid_t pid);
		};		
	}
}


#endif

