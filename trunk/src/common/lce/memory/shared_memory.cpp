#include <new>
#include <iostream>
#include <time.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "shared_memory.h"

using namespace std;

namespace lce
{
	namespace memory
	{
		void* SharedMemory::get_shm(int key, int shmsize, int shmflag, int& shmid, bool& isnew)
		{
			
			if((shmid = shmget(key, shmsize, IPC_CREAT | IPC_EXCL | shmflag)) < 0)	
			{
				if (errno != EEXIST)
				{
		            cerr<<"getshm failed,error is "<<strerror(errno)<<endl;
					return NULL;
				}

				shmflag &= ~ (IPC_CREAT | IPC_EXCL);
				if((shmid = shmget(key, shmsize , shmflag)) < 0)				
				{
					cerr<<"try get already existed shm failed, key="<<key<<endl;
					return NULL;
				}

		        isnew = false;
			}
		    else
	        {
	            isnew = true;
	        }
		    
		    char *p = NULL;   
			if((p=(char *)shmat(shmid, 0, 0)) < 0 )
			{
				cerr<<"attache_shm failed ,error is "<<strerror(errno)<<", key="<<key<<endl;
				return NULL;
			}

			return p;
		}

		void* SharedMemory::get_shm(const std::string& pathname, int projid, int shmsize,int shmflg, int& shmid, bool& isnew)
		{
			key_t key = ftok(pathname.c_str(), projid);
			return get_shm(key, shmsize, shmflg, shmid, isnew);
		}

		int SharedMemory::attach_shm(int shmid,char **p)
		{
			if((*p=(char *)shmat(shmid, 0, 0)) < 0 )
			{
				cerr<<"atach shm failed,error is "<<strerror(errno)<<endl;
				return -1;
			}

			return 0;
		}

		int SharedMemory::detach_shm(char *p)
		{
			if(shmdt(p)<0)
			{
				cerr<<"detach shm failed,error is "<<strerror(errno)<<endl;
				return -1;
			}
			return 0;
		}

		int SharedMemory::remove_shm(int shmid)
		{
			if(shmctl(shmid,IPC_RMID,0)<0)
			{
				cerr<<"remove shm failed,error is "<<strerror(errno)<<endl;
				return -1;
			}
			
			return 0;
		}	

		void* SharedMemory::operator new(size_t blocksize, string pathname, int projid)
		{
			int shmid = -1;
	        bool isnew = false;
		    int shmsize = blocksize; //  blocksize is callback in from class traitor info

	        SharedMemory* sm = (SharedMemory*)get_shm(pathname, projid, shmsize, 0666, shmid, isnew);
			if(sm == NULL)
	        {
		    	cerr << "SharedMemory::new(), get (shmget) failed ,error is "<<strerror(errno)<<endl;
		    	throw bad_alloc();
	        }

			sm->_shmid = shmid;
			sm->_key = ftok(pathname.c_str(), projid);;
		    sm->_is_new = isnew;
		    
			return sm;
		}

		void* SharedMemory::operator new(size_t blocksize,  int key)
		{
			int shmid = -1;
	        bool isnew = false;
		    int shmsize = blocksize; //  blocksize is callback in from class traitor info

	        SharedMemory* sm = (SharedMemory*)get_shm(key, shmsize, 0666, shmid, isnew);
			if(sm == NULL)
	        {
		    	cerr << "SharedMemory::new(), get (shmget) failed ,error is "<<strerror(errno)<<endl;
		    	throw bad_alloc();
	        }

			sm->_shmid = shmid;
			sm->_key = key;
		    sm->_is_new = isnew;
		    
			return sm;
		}

		void SharedMemory::operator delete(void *p)
		{
		       shmdt(p);
		}	
	}
}
