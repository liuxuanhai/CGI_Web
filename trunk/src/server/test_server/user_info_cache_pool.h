#ifndef _TEST_SERVER_USER_INFO_CACHE_POOL_H_
#define _TEST_SERVER_USER_INFO_CACHE_POOL_H_
#include <string>
#include "util/logger.h"
#include <ext/hash_map>
#include "memory/array_object_pool.h"
#include "dao.h"

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

typedef struct user_info_cache_node_t
{
    DaoUserInfo _user_info;
    user_info_cache_node_t* _prev;
    user_info_cache_node_t* _next;
} user_info_cache_node_t;


class UserInfoCachePool
{
public:
	typedef user_info_cache_node_t CacheNode;
    typedef __gnu_cxx::hash_map<uint32_t, CacheNode*>  CacheMap;//id->user_info
    typedef lce::memory::ArrayObjectPool<user_info_cache_node_t> CachePool;

public:
    UserInfoCachePool(size_t capacity);
    ~UserInfoCachePool();

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

    int add(const DaoUserInfo& stUserInfo);
    DaoUserInfo* get(uint32_t id);
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

