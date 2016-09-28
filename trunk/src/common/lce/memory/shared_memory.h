
#ifndef _SHARED_MEMORY_H_
#define _SHARED_MEMORY_H_ 

#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string>

namespace lce
{
    namespace memory
    {
        class SharedMemory
        {
        public:
            static void * get_shm(int key,int shmsize,int shmflg, int& shmid, bool& isnew);
            static void * get_shm(const std::string& pathname, int projid, int shmsize,int shmflg, int& shmid, bool& isnew);
            static int attach_shm(int shmid,char **p);
            static int detach_shm(char *p);
            static int remove_shm(int shmid);

        public:
            SharedMemory(){}
            bool is_new(){ return _is_new;}
            int get_shmid(){ return _shmid;}
            int get_shmkey(){ return _key;}
            
            void *operator new(size_t blocksize,int key) ;  
            void* operator new(size_t blocksize, std::string pathname, int projid);
            void operator delete(void *);

        private:
            int _shmid;
            int _key;
            bool _is_new; // if shm is created first time
            
        };
    }
}


#endif
