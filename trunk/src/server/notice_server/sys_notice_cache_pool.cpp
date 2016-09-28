#include "sys_notice_cache_pool.h"
#include "global_var.h"
#include <stdarg.h>
#include <lce/util/md5.h>
IMPL_LOGGER(SysNoticeCachePool, _logger);
uint64_t SysNoticeCachePool::generate_id(uint64_t md5A, ...)
{
	int j = 0;
	std::ostringstream oss;
	oss.str("");
	va_list vl;
	va_start(vl, md5A);
	while (j != -1)
	{

		j = va_arg(vl, uint64_t); //将当前参数转换为int类型
		oss << j;
	}
	md5 m_md5;
	LOG4CPLUS_DEBUG(_logger, "generate_id: " << oss.str());
	va_end(vl);

	return m_md5.hash64(oss.str().c_str(), strlen(oss.str().c_str()));
}
SysNoticeCachePool::SysNoticeCachePool(size_t capacity) :
		_capacity(capacity), _cache_map_A(), _cache_pool(_capacity), _lru_head(NULL), _lru_tail(NULL)
{
}

SysNoticeCachePool::~SysNoticeCachePool()
{
}

int SysNoticeCachePool::init()
{
	int rv = _cache_pool.init();
	if (rv < 0)
	{
		LOG4CPLUS_ERROR(_logger, "init SysNoticeCachePool failed!! capacity: " << _capacity);
		return -1;
	}

	return 0;
}

bool SysNoticeCachePool::exists(uint64_t pa_appid_md5, uint64_t create_ts)
{
	CacheMapA::iterator iterA = _cache_map_A.find(pa_appid_md5);
	if (iterA != _cache_map_A.end())
	{
		CacheMapB::iterator iterB = (CacheMapB(iterA->second)).find(create_ts);
		if (iterB != (CacheMapB(iterA->second)).end())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}

int SysNoticeCachePool::add(const DaoSysNotice& stSysNotice)
{
	//if exists, move to lru head
	CacheMapA::iterator iterA = _cache_map_A.find(stSysNotice.m_pa_appid_md5);
	if (iterA != _cache_map_A.end())
	{
		CacheMapB *_tmp_cache_map_B = (CacheMapB *)&(iterA->second);
		CacheMapB::iterator iterB = _tmp_cache_map_B->find(stSysNotice.m_create_ts);

		if (iterB != _tmp_cache_map_B->end())
		{
			lru_move_to_head(iterB->second);
			return 0;
		}
	}

	//judge if space enough
	int left_size = capacity() - size();
	uint32_t cfg_min_left = g_server->config().get_int_param("SYS_NOTICE_CACHE_POOL", "min_left");
	uint32_t cfg_purge_num = g_server->config().get_int_param("SYS_NOTICE_CACHE_POOL", "purge_num");

	if ((uint32_t) left_size < cfg_min_left)
	{
		LOG4CPLUS_DEBUG(_logger,
				"archive cache pool need to be purged..." <<", left size:" << left_size <<", min left num :" << cfg_min_left <<", purge num:" << cfg_purge_num);

		lru_purge(cfg_purge_num);
	}

	CacheNode* pnode = _cache_pool.construct();
	assert(pnode);
	pnode->_sys_notice = stSysNotice;

	std::pair<CacheMapA::iterator, bool> retA;
	std::pair<CacheMapB::iterator, bool> retB;

	iterA = _cache_map_A.find(stSysNotice.m_pa_appid_md5);
	if (iterA != _cache_map_A.end())
	{
		retB = ((CacheMapB) (iterA->second)).insert(CacheMapB::value_type(stSysNotice.m_create_ts, pnode));
		assert(retB.second);
	}
	else
	{
		CacheMapB _tmp_cache_map_B;
		retB = _tmp_cache_map_B.insert(CacheMapB::value_type(stSysNotice.m_create_ts, pnode));
		assert(retB.second);

		retA = _cache_map_A.insert(CacheMapA::value_type(stSysNotice.m_pa_appid_md5, _tmp_cache_map_B));
		assert(retA.second);

	}

	//update lruzz
	lru_add_to_head(pnode);

	return 0;
}

DaoSysNotice* SysNoticeCachePool::get(uint64_t pa_appid_md5, uint64_t create_ts)
{
//	LOG4CPLUS_DEBUG(_logger, _cache_map.size());
	CacheMapA::iterator iterA = _cache_map_A.find(pa_appid_md5);
	if (iterA != _cache_map_A.end())
	{
		CacheMapB::iterator iterB = (CacheMapB(iterA->second)).find(create_ts);
		if (iterB != (CacheMapB(iterA->second)).end())
		{
			CacheNode* pnode = iterB->second;

			//update_lru
			lru_move_to_head(pnode);

			return &(pnode->_sys_notice);
		}
		else
		{
			return NULL;
		}
	}
	return NULL;

}

void SysNoticeCachePool::remove(uint64_t pa_appid_md5, uint64_t create_ts)
{

	CacheMapA::iterator iterA = _cache_map_A.find(pa_appid_md5);
	if (iterA != _cache_map_A.end())
	{

		CacheMapB * _tmp_cache_map_B = &(iterA->second);
		CacheMapB::iterator iterB = (*_tmp_cache_map_B).find(create_ts);
		if (iterB != (*_tmp_cache_map_B).end())
		{
			CacheNode* pnode = iterB->second;
			lru_delete_node(pnode);

			(*_tmp_cache_map_B).erase(create_ts);
			if ((*_tmp_cache_map_B).size() == 0)
			{
				_cache_map_A.erase(pa_appid_md5);
			}

			_cache_pool.destroy(pnode);
		}
		else
		{
			return;
		}
	}
	return;
}

void SysNoticeCachePool::lru_add_to_head(CacheNode * pnode)
{
	assert(pnode);

	if (_lru_head == NULL)
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

void SysNoticeCachePool::lru_move_to_head(CacheNode * pnode)
{
	assert(pnode);
	assert(_lru_head);

	if (pnode == _lru_head)
	{
		return;
	}

	if (pnode == _lru_tail)
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

void SysNoticeCachePool::lru_delete_node(CacheNode* pnode)
{
	assert(pnode);
	assert(_lru_head);

	if (_lru_head == _lru_tail)
	{
		assert(pnode == _lru_head);
		_lru_head = _lru_tail = NULL;
	}
	else if (pnode == _lru_head)
	{
		_lru_head = pnode->_next;
		pnode->_next->_prev = NULL;
	}
	else if (pnode == _lru_tail)
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

size_t SysNoticeCachePool::lru_purge(size_t num_to_purge)
{

	if (_lru_tail == NULL)
	{
		LOG4CPLUS_DEBUG(_logger, "nothing to purge in archive_cache..");
		return 0;
	}

	CacheNode *pnode = _lru_tail;

	size_t i = 0;
	for (; i < num_to_purge && pnode != NULL; i++)
	{
		CacheNode* tmp = pnode;
		pnode = pnode->_prev;

		lru_delete_node(tmp);

		CacheMapA::iterator iterA = _cache_map_A.find(tmp->_sys_notice.m_pa_appid_md5);
		if (iterA != _cache_map_A.end())
		{

			CacheMapB * _tmp_cache_map_B = &(iterA->second);
			CacheMapB::iterator iterB = (*_tmp_cache_map_B).find(tmp->_sys_notice.m_create_ts);
			if (iterB != (*_tmp_cache_map_B).end())
			{

				(*_tmp_cache_map_B).erase(tmp->_sys_notice.m_create_ts);
				if ((*_tmp_cache_map_B).size() == 0)
				{
					_cache_map_A.erase(tmp->_sys_notice.m_pa_appid_md5);
				}

			}
			else
			{
				LOG4CPLUS_ERROR(_logger, "lru_purge: pa_appid_md5=" << tmp->_sys_notice.m_pa_appid_md5<<", create_ts="<<tmp->_sys_notice.m_create_ts);
			}

		}


		_cache_pool.destroy(tmp);
	}

	return i;
}

