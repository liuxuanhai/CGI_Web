#ifndef _NOTICE_SERVER_SYS_NOTICE_CACHE_POOL_H_
#define _NOTICE_SERVER_SYS_NOTICE_CACHE_POOL_H_
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

typedef struct sys_notice_cache_node_t
{
    DaoSysNotice _sys_notice;
    sys_notice_cache_node_t* _prev;
    sys_notice_cache_node_t* _next;
} sys_notice_cache_node_t;


class SysNoticeCachePool
{
public:
	typedef sys_notice_cache_node_t CacheNode;
	// key uint64_t p_appid_md5 , uint64_t create_ts
//	typedef __gnu_cxx::hash_map<uint64_t, uint64_t,uint64_t,CacheNode*>  CacheMap;//id->sys_notice
	typedef __gnu_cxx::hash_map<uint64_t,CacheNode*>  CacheMapB;//id->sys_notice
	typedef __gnu_cxx::hash_map<uint64_t,CacheMapB >  CacheMapA;//id->sys_notice


    typedef lce::memory::ArrayObjectPool<sys_notice_cache_node_t> CachePool;

public:
    SysNoticeCachePool(size_t capacity);
    ~SysNoticeCachePool();

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
        //return _cache_map_B.size();
    	return 0;
    }

    bool exists(uint64_t pa_appid_md5,uint64_t create_ts);

    int add(const DaoSysNotice& stSysNotice);
    DaoSysNotice* get(uint64_t pa_appid_md5, uint64_t create_ts);
    void remove(uint64_t pa_appid_md5, uint64_t create_ts);

    uint64_t generate_id(uint64_t md5A, ...);

private:
    void lru_add_to_head(CacheNode* pnode);
    void lru_move_to_head(CacheNode* pnode);
    void lru_delete_node(CacheNode* pnode);
    size_t lru_purge(size_t num_to_purge);

private:
    size_t     _capacity;
    CacheMapA   _cache_map_A;
	CachePool  _cache_pool;
    CacheNode* _lru_head;
    CacheNode* _lru_tail;

    DECL_LOGGER(_logger);
};

#endif

