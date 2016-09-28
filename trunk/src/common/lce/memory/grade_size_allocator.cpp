#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grade_size_allocator.h"

namespace lce
{
    namespace memory
    {
        size_t GradeSizeAllocator::power(size_t bytes)
        {
            size_t index = 1;

            if(bytes==0)
            {
                return 0;
            }
            
            bytes--;
            
            while(bytes >>= 1)
            {
                index++;
            }

            return index;
        }
		
        size_t GradeSizeAllocator::freelist_index(size_t bytes)
        {
            size_t index = 1;

            if(bytes==0)
            {
                return _SMALLEST_POWER;
            }
            
            bytes--;
            
            while(bytes >>= 1)
            {
                index++;
            }
            
            if (index < _SMALLEST_POWER) 
            {
                index = _SMALLEST_POWER;
            }

            assert(index <= _LARGEST_POWER);
            
            return index;        
        }

        int  GradeSizeAllocator::init(int size)
        {
            if((_MAX_BYTES <=0) || (_ALIGN <=0 ))
            {
                return -1;
            }

            _NFREELISTS = _LARGEST_POWER+1;

            _free_list = new Obj* volatile[_NFREELISTS];

            for(int i=0; i < _NFREELISTS; i++)
            {
                _free_list[i] = 0;
            }

            int real_size = size;
            _start_free = (char*)malloc(real_size);
            if(_start_free == NULL)
            {
                return -1;
            }
            else
            {
                _base_ptr       =  _start_free;
                _end_free       =  _start_free + real_size;
                _heap_size     = real_size;

                for(uint32_t i=_SMALLEST_POWER; i <= _LARGEST_POWER; i++)
                {
                    for(int32_t j=0; j<(1<<(_LARGEST_POWER-i)); j++)
                    {
                        void* p = allocate(1<<i);
                        if(p == NULL)
                        {
                            return -1;
                        }
                        deallocate(p, 1<<i);
                    }
                }
                
                return _heap_size;
            }
        }

        char* GradeSizeAllocator::_chunk_alloc(size_t size, int& nobjs)
        {
            char* result;
            size_t total_bytes = size * nobjs;
            size_t bytes_left = _end_free - _start_free;

            if (bytes_left >= total_bytes)
            {
                result = _start_free;
                _start_free += total_bytes;
                return result;
            }
            else if (bytes_left >= size)
            {
                nobjs = (int)(bytes_left/size);
                total_bytes = size * nobjs;
                result = _start_free;
                _start_free += total_bytes;
                return result;
            }
            else
            {
                return NULL;
            }
        }

        void* GradeSizeAllocator::_refill(size_t n)
        {
            int nobjs = 10;
            char* chunk = _chunk_alloc(n, nobjs);
            

            if(chunk == NULL)
            {
                return NULL;
            }

            if (1 == nobjs) 
            {
                return chunk;
            }

            Obj* volatile* my_free_list;
            Obj* result;
            Obj* current_obj;
            Obj* next_obj;

            my_free_list = _free_list + freelist_index(n);
            result = (Obj*)chunk;
            *my_free_list = next_obj = (Obj*)(chunk + n);
            for (int i = 1; ; i++)
            {
                current_obj = next_obj;
                next_obj = (Obj*)((char*)next_obj + n);

                if (nobjs - 1 == i)
                {
                    current_obj -> _free_list_link = 0;
                    break;
                }
                else
                {
                    current_obj -> _free_list_link = next_obj;
                }
            }

            return result;
        }

        void* GradeSizeAllocator::allocate(size_t n)
        {
            void* ret = 0;

            if (0 == n || n > (size_t) _MAX_BYTES)
            {
                return NULL;
            }
            else
            {
                size_t list_index = freelist_index(n);
                
                Obj* volatile* _my_free_list = _free_list + list_index;

                Obj*  _result = *_my_free_list;
                if (_result == 0)
                {
                    size_t to_fill = 1<<list_index;
                    ret = _refill(to_fill); 
                }
                else
                {
                    *_my_free_list = _result -> _free_list_link;
                    ret = _result;
                }
            }

            return ret;
        };

        void GradeSizeAllocator::deallocate(void* p, size_t n)
        {
            if (NULL == p || n > (size_t) _MAX_BYTES)
            {
                assert(false);
                return;
            }
            else
            {
                Obj* volatile*  _my_free_list = _free_list + freelist_index(n);
                Obj* q = (Obj*)p;

                q -> _free_list_link = *_my_free_list;
                *_my_free_list = q;
            }
        }


        size_t  GradeSizeAllocator::remain()
        {
            size_t total = 0;
            
            for(uint32_t i=_SMALLEST_POWER; i <= _LARGEST_POWER; i++)
            {
                Obj* volatile  _my_free_list = _free_list[i];
                while(_my_free_list != 0)
                {
                    total+= 1<<i;
                    _my_free_list = _my_free_list -> _free_list_link;
                }
            }
            return total;
        }
    }
}


