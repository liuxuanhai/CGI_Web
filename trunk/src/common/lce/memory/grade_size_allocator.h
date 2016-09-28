#ifndef _GRADE_SIZE_ALLOCATOR_H_
#define _GRADE_SIZE_ALLOCATOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

namespace lce
{
    namespace memory
    {
        class GradeSizeAllocator
        {
        private:
            union Obj
            {
                union Obj* _free_list_link;
                char       _client_data[1];
            };


        public:
            GradeSizeAllocator(int smallest_power = 10, int largest_power = 22)  // 1KB -----  4MB, default
            : _SMALLEST_POWER(smallest_power)
            , _LARGEST_POWER(largest_power)
            , _ALIGN(1<<_SMALLEST_POWER)
            , _MAX_BYTES(1<<_LARGEST_POWER)
            , _NFREELISTS(0)
            , _free_list(NULL)
            , _base_ptr(NULL)
            , _heap_size(0)
            , _start_free(NULL)
            , _end_free(NULL)

            {

            }

            ~GradeSizeAllocator()
            {
                // total src data memory
                if(_base_ptr)
                {
                    delete _base_ptr;
                    _base_ptr = NULL;
                }

                // data memeory manager unit
                if(_free_list)
                {
                    delete _free_list;
                    _free_list = NULL;
                }
            }

            // common interface
            int  init(int size);
            void* allocate(size_t _n);
            void deallocate(void* _p, size_t _n);

            static size_t power(size_t _bytes);

            //self info
            size_t  remain();
            size_t  heap_size(){return _heap_size;}
            size_t  max_bytes(){return _MAX_BYTES;}
            size_t  min_bytes(){return _ALIGN;}
            size_t  freelist_num(){return _LARGEST_POWER - _SMALLEST_POWER + 1;}
            size_t  smallest_power(){return _SMALLEST_POWER;}
            size_t  largest_power(){return _LARGEST_POWER;}

        public:
            const size_t _SMALLEST_POWER;
            const size_t _LARGEST_POWER;    
            
            const int _ALIGN;
            const int _MAX_BYTES;
            int       _NFREELISTS;


        private:
            size_t _round_up(size_t _bytes)
            {
                return (((_bytes) + (size_t) _ALIGN-1) & ~((size_t) _ALIGN - 1));
            }

            // Returns an object of size _n, and some additional same obejects maybe add to  _n free list.
            void* _refill(size_t _n);

            // Allocates a chunk for nobjs of size size, nobjs may be reduced if the not enough
            char* _chunk_alloc(size_t _size, int& _nobjs);

            size_t  freelist_index(size_t _bytes);

        private:
            Obj* volatile*  _free_list;
            char* _base_ptr;
            size_t _heap_size;
            char* _start_free;
            char* _end_free;
        } ;
    }
}

#endif

