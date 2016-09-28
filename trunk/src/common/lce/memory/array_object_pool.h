#ifndef _ARRAY_OBJECT_POOL_H_
#define _ARRAY_OBJECT_POOL_H_

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <new>

namespace lce
{
	namespace memory
	{
		template<class ObjectType>
		class ArrayObjectPool
		{
			typedef ObjectType value_type;

			union Obj
			{
				Obj* _free_list_link;
				char _client_data[1];
			};

		public:
			explicit ArrayObjectPool(int capacity)
				: _values(NULL)
				  , _capacity(capacity)
				  , _free_num(_capacity)
			{

			}
			
			ArrayObjectPool()
			    :_values(NULL),
			    _capacity(0),
			    _free_num(0)
			{

			}
			    

			~ArrayObjectPool()
			{
				if(_values)
				{
					free(_values);
					_values = NULL;
				}
			}

			int init(int capacity)
			{
				_capacity = capacity;
				_free_num = capacity;
				return init();
			}

			int32_t init()
			{
				assert(_capacity > 0);
				_values = (value_type*)malloc(sizeof(value_type) * _capacity);
				if(_values == NULL)
				{
					return -1;
				}

				_free_head._free_list_link = (Obj*)( _values);
				for(int i=0; i < _capacity - 1; i++)
				{
					((Obj*)(_values + i))->_free_list_link =(Obj*)( _values+i+1);
				}

				((Obj*)(_values + (_capacity - 1)))->_free_list_link = 0;

				return 0;
			}

			value_type* construct()
			{
				Obj* free_node = _free_head._free_list_link;
				if(free_node == 0)
				{
					return NULL;
				}
				else
				{
					_free_head._free_list_link = free_node->_free_list_link;
					_free_num--;
					return new(free_node)value_type();
				}     
			}

			template <class param1_type>
		    value_type* construct(param1_type param1)
			{
				Obj* free_node = _free_head._free_list_link;
				if(free_node == 0)
				{
					return NULL;
				}
				else
				{
					_free_head._free_list_link = free_node->_free_list_link;
					_free_num--;
					return new(free_node)value_type( param1 );
				}      
			}

			template <class param1_type, class param2_type>
			value_type* construct(param1_type param1, param2_type param2)
			{
				Obj* free_node = _free_head._free_list_link;
				if(free_node == 0)
				{
					return NULL;
				}
				else
				{
					_free_head._free_list_link = free_node->_free_list_link;
					_free_num--;
					return new(free_node)value_type( param1, param2 );
				}      
			}

			void  destroy(value_type* node)
			{
				assert((node>=_values)&&(node<(_values+_capacity)));
				node->~value_type();
				((Obj*)(node))->_free_list_link = _free_head._free_list_link;
				_free_head._free_list_link = ((Obj*)(node));
				_free_num++;
			}

			int capacity()
			{
				return _capacity;
			}

			int free_objs_num()
			{
				return _free_num;
			}

			int index(value_type* node)
			{
				return node - _values;
			}

			value_type& operator[](uint32_t pos)
			{
				return _values[pos];
			}

			private:
			value_type*   _values;
			int           _capacity;
			Obj           _free_head;
			int           _free_num;
		};
	}
}

#endif

