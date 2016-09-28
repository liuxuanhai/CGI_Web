#ifndef _TEST_SERVER_FSM_CONTAINER_H_
#define _TEST_SERVER_FSM_CONTAINER_H_
#include <ext/hash_map>
#include "memory/array_object_pool.h"
#include "util/logger.h"


template <class F>
class FsmContainer
{
public:
	enum FsmType
	{
		FSM_TYPE_PREAUTH = 1,
		FSM_TYPE_RESOURCE = 2,
		FSM_TYPE_BUSINESS = 3,
	};

	static std::string fsm_type_string(int iFsmType)
	{
		switch(iFsmType)
        {
            case FSM_TYPE_PREAUTH:
                return "FSM_TYPE::PREAUTH";

			case FSM_TYPE_RESOURCE:
				return "FSM_TYPE::RESOURCE";

			case FSM_TYPE_BUSINESS:
				return "FSM_TYPE::BUSINNESS";
                
            default:
                return "FSM_TYPE::UNKNOWN";
        } 

        return "";
	}

public:
	typedef __gnu_cxx::hash_map<uint32_t, F*> FsmMap;

public:
	FsmContainer(int capacity): _pool(capacity), _next_id(0)
	{
 
	}

	virtual ~FsmContainer()
	{

	}

	int init();
	F* new_fsm();
	F* find_fsm(uint32_t id);
	void del_fsm(F* fsm); 

private:
	lce::memory::ArrayObjectPool<F> _pool;
	FsmMap _map;

	uint32_t _next_id;
};

template<class F>
int FsmContainer<F>::init()
{
	return _pool.init();
}

template<class F>
F* FsmContainer<F>::new_fsm()
{
    F* pfsm= _pool.construct();
    if(!pfsm) 
    {   
        return NULL;
    }   

    pfsm->_id = ++_next_id;
    _map.insert(make_pair(pfsm->_id, pfsm));

    return pfsm;
}

template<class F>
F* FsmContainer<F>::find_fsm(uint32_t id) 
{	
    class FsmMap::iterator iter = _map.find(id);
    if(iter == _map.end())
    {   
        return NULL;
    }   

    return iter->second;
}

template<class F>
void FsmContainer<F>::del_fsm(F* pfsm)
{
	
    class FsmMap::iterator iter = _map.find(pfsm->_id);
    if(iter != _map.end())
    {
        _pool.destroy(iter->second);
        _map.erase(iter);
    }

	return;
}

#endif
