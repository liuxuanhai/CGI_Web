#ifndef _MSG_SERVER_MSG_CACHE_POOL_H_
#define _MSG_SERVER_MSG_CACHE_POOL_H_
#include <string>
#include "util/logger.h"
#include <ext/hash_map>
#include "memory/array_object_pool.h"
#include "msg_content.h"

struct StringHash
{
    size_t __stl_hash_string(const char* __s) const
    {
        unsigned long __h = 0;
        for(; *__s; ++__s)
            __h = 5*__h + *__s;

        return size_t(__h);
    }
	
    size_t operator()(const std::string& str) const
    {
        return __stl_hash_string(str.c_str());
    }
};

typedef struct msg_content_cache_node_t
{
    MsgContent _msg_content;
    msg_content_cache_node_t* _prev;
    msg_content_cache_node_t* _next;
} msg_content_cache_node_t;


class MsgContentCachePool
{
public:
	typedef msg_content_cache_node_t CacheNode;
    typedef __gnu_cxx::hash_map<uint32_t, CacheNode*>  CacheMap;//id->user_info
    typedef lce::memory::ArrayObjectPool<msg_content_cache_node_t> CachePool;

public:
    MsgContentCachePool(size_t capacity);
    ~MsgContentCachePool();

public:
    int init();

    bool empty() const
    {
        return _lru_head == NULL;
    }
    size_t capacity() const
    {
        return _capacity;
    }
    size_t size() const
    {
        return _cache_map.size();
    }

    bool exists(uint32_t id);

    int add(const MsgContent& stUserInfo);
    MsgContent* get(uint32_t id);
    void remove(uint32_t id);

private:
    void lru_add_to_head(CacheNode* pnode);
    void lru_move_to_head(CacheNode* pnode);
    void lru_delete_node(CacheNode* pnode);
    size_t lru_purge(size_t num_to_purge);

private:
    size_t     _capacity;
    CacheMap   _cache_map;
	CachePool  _cache_pool;
    CacheNode* _lru_head;
    CacheNode* _lru_tail;

    DECL_LOGGER(_logger);
};

#endif

