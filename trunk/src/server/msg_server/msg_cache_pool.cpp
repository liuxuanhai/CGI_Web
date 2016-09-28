#include "msg_cache_pool.h"
#include "global_var.h"

IMPL_LOGGER(MsgContentCachePool, _logger);

MsgContentCachePool::MsgContentCachePool(size_t capacity)
    : _capacity(capacity)
    , _cache_map()
    , _cache_pool(_capacity)
    , _lru_head(NULL)
    , _lru_tail(NULL)
{
}

MsgContentCachePool::~MsgContentCachePool()
{
}

int MsgContentCachePool::init()
{
    int rv = _cache_pool.init();
    if(rv < 0)
    {
        LOG4CPLUS_ERROR(_logger, "init MsgContentCachePool failed!! capacity: " << _capacity);
        return -1;
    }

    return 0;
}

bool MsgContentCachePool::exists(uint32_t id)
{
    CacheMap::iterator iter = _cache_map.find(id);
    return iter != _cache_map.end();
}

int MsgContentCachePool::add(const MsgContent& stUserInfo)
{
	//if exists, move to lru head
	CacheMap::iterator iter = _cache_map.find(stUserInfo.m_id);
    if(iter != _cache_map.end())
    {
		lru_move_to_head(iter->second);
		return 0;
	}


	//judge if space enough
    int left_size = capacity() - size();
	uint32_t cfg_min_left = g_server->config().get_int_param("USER_INFO_CACHE_POOL", "min_left");
	uint32_t cfg_purge_num = g_server->config().get_int_param("USER_INFO_CACHE_POOL", "purge_num"); 

    if((uint32_t)left_size < cfg_min_left)
    {
        LOG4CPLUS_DEBUG(_logger, "archive cache pool need to be purged..."
						<<", left size:" << left_size 
						<<", min left num :" << cfg_min_left
						<<", purge num:" << cfg_purge_num);
		
        lru_purge(cfg_purge_num);
    }

	
    CacheNode* pnode = _cache_pool.construct();
    assert(pnode);
    pnode->_msg_content = stUserInfo;

    std::pair<CacheMap::iterator, bool> ret;
    ret = _cache_map.insert(CacheMap::value_type(stUserInfo.m_id, pnode));
    assert(ret.second);

	//update lru
    lru_add_to_head(pnode);

    return 0;
}

MsgContent* MsgContentCachePool::get(uint32_t id)
{
   	CacheMap::iterator it = _cache_map.find(id);
    if(it == _cache_map.end())
    {
		return NULL;
	}
        
	CacheNode* pnode = it->second;

	//update_lru
	lru_move_to_head(pnode);

    return &(pnode->_msg_content);
}

void MsgContentCachePool::remove(uint32_t id)
{
    CacheMap::iterator it = _cache_map.find(id);

    if(it == _cache_map.end())
    {
		return;
	}
        
    CacheNode* pnode = it->second;
    lru_delete_node(pnode);
    _cache_map.erase(id);
    _cache_pool.destroy(pnode);
}

void MsgContentCachePool::lru_add_to_head(CacheNode * pnode)
{
    assert(pnode);

    if(_lru_head == NULL)
    {
        pnode->_prev = NULL;
        pnode->_next = NULL;
        _lru_head = pnode;
        _lru_tail = pnode;
    }
    else
    {
        pnode->_prev = NULL;
        pnode->_next = _lru_head;
        _lru_head->_prev = pnode;
        _lru_head = pnode;
    }
}

void MsgContentCachePool::lru_move_to_head(CacheNode * pnode)
{
    assert(pnode);
    assert(_lru_head);

    if(pnode == _lru_head)
    {
		return;
	}
        
    if(pnode == _lru_tail)
    {
        _lru_tail->_prev->_next = NULL;
        _lru_tail = _lru_tail->_prev;
    }
    else
    {
        pnode->_prev->_next = pnode->_next;
        pnode->_next->_prev = pnode->_prev;
    }

    lru_add_to_head(pnode);
}

void MsgContentCachePool::lru_delete_node(CacheNode* pnode)
{
    assert(pnode);
    assert(_lru_head);

    if(_lru_head == _lru_tail)
    {
        assert(pnode == _lru_head);
        _lru_head = _lru_tail = NULL;
    }
    else if(pnode == _lru_head)
    {
        _lru_head = pnode->_next;
        pnode->_next->_prev = NULL;
    }
    else if(pnode == _lru_tail)
    {
        _lru_tail = pnode->_prev;
        pnode->_prev->_next = NULL;
    }
    else
    {
        pnode->_prev->_next = pnode->_next;
        pnode->_next->_prev = pnode->_prev;
    }
}

size_t MsgContentCachePool::lru_purge(size_t num_to_purge)
{
    if(_lru_tail == NULL)
    {
        LOG4CPLUS_DEBUG(_logger, "nothing to purge in archive_cache..");
        return 0;
    }

    CacheNode *pnode = _lru_tail;

    size_t i = 0;
    for(; i < num_to_purge && pnode != NULL; i++)
    {
        CacheNode* tmp = pnode;
        pnode = pnode->_prev;

        lru_delete_node(tmp);
		_cache_map.erase(tmp->_msg_content.m_id);
        _cache_pool.destroy(tmp);
    }

    return i;
}

