#include "pay_common_util.h"
#include "util/md5.h"
#include <stdio.h>
#include "libmemcached/memcached.h"
#include <sys/time.h>

inline uint64_t GetTickCountUsec()
{
	timeval tv;
	gettimeofday(&tv, 0);
	return tv.tv_sec * 1000 * 1000 + tv.tv_usec;
}


std::string str_replace(std::string& srcstr, const std::string& substr, const std::string& repstr)
{
	for(std::string::size_type pos(0); pos != std::string::npos; pos += repstr.length())
	{
		if((pos=srcstr.find(substr, pos)) != std::string::npos)
			srcstr.replace(pos, substr.length(), repstr);
		else
			break;
	}
	return srcstr;
}

std::string XML_ESCAPE(std::string& srcstr)
{
	str_replace(srcstr, "&", "&amp;");
	str_replace(srcstr, "<", "&lt;");
	str_replace(srcstr, ">", "&gt;");					
	str_replace(srcstr, "'", "&apos;");
	str_replace(srcstr, "\"", "&quot;");
	
	return srcstr;
}

std::string genSignString(std::map<std::string, std::string>& mapParams, const std::string& key)
{
	std::vector<std::string> vecKeys;
	for(std::map<std::string, std::string>::const_iterator iter = mapParams.begin()
		; iter != mapParams.end()
		; ++iter)
	{
		if(!iter->second.empty())
		{
			vecKeys.push_back(iter->first);
		}
	}

	std::sort(vecKeys.begin(), vecKeys.end());

	std::string strSortedParams;
	for(size_t i=0; i!=vecKeys.size(); ++i)
	{
		//for sign
		const std::string& strKey = vecKeys[i];
		const std::string& strValue = mapParams[strKey];
		if(i != 0)
		{
			strSortedParams += "&";
		}
		strSortedParams += (strKey + "=" + strValue);		
	}
	
	strSortedParams += ("&key=" + key);

	md5 m_md5;
	std::string result =m_md5.hash(strSortedParams);
	::transform(result.begin(), result.end(), result.begin(), ::toupper);
	return result;
	
}

std::string genRandomString(int length)
{		
	std::string randstr;
	char tmp[length + 1];
	int i = 0;
	for(; i<length; i++)
	{
		srand((GetTickCountUsec() >> i) ^ 77);
		int type = rand() % 3;				

		switch(type)
		{
			case 0:
				tmp[i] = '0' + rand() % 10;								
				break;
			case 1:
				tmp[i] = 'A' + rand() % 26;																
				break;
			case 2:
				tmp[i] = 'a' + rand() % 26;							
				break;
			default:								
				break;
		}				
	}

	tmp[i] = '\0';
	return randstr = tmp;
}

std::string genLocalTimeString(int expire_s)
{
	std::string strTime;
	struct tm* local;
	time_t t = time(NULL) + expire_s;
	local = localtime(&t);
	char tmp[16];
	sprintf(tmp, "%04d%02d%02d%02d%02d%02d"
				, local->tm_year + 1900
				, local->tm_mon + 1
				, local->tm_mday
				, local->tm_hour
				, local->tm_min
				, local->tm_sec);
	strTime = tmp;
	return strTime;
}

std::string genOutTradeNoString()
{
	std::string strLocalTime = genLocalTimeString(0);	
	
	int length = 18;
	char suffix[length + 1];

	int i = 0;
	for(; i < length; i++)
	{
		srand((GetTickCountUsec() >> i) ^ 77);
		suffix[i] = '0' + rand() % 10;
	}
	suffix[i] = '\0';

	return strLocalTime + suffix;
}

std::string genRedPackBillNoString(const std::string& mch_id)
{
	std::string strMchidLocalTime = mch_id + genLocalTimeString(0);	

	int length = 4;
	char suffix[length + 1];
	
	int i = 0;
	for(; i < length; i++)
	{
		srand((GetTickCountUsec() >> i) ^ 77);
		suffix[i] = '0' + rand() % 10;
	}
	suffix[i] = '\0';

	return strMchidLocalTime + suffix;
}

bool memcacheSessionAdd(const std::string& ip, uint16_t port, 
								const std::string& key, const std::string& value, 
								uint32_t expire_ts, std::string& strErrMsg)
{
	if(key.empty() || value.empty())
	{
		strErrMsg = "empty key or value!!";
		return false;
	}

	//connect cache
	memcached_return rc;
	memcached_st *memc = memcached_create(NULL);
	memcached_server_st *server = memcached_server_list_append(NULL, ip.c_str(), port, &rc);
    rc = memcached_server_push(memc,server);
    if(MEMCACHED_SUCCESS != rc)
    {
         strErrMsg = "memcached_server_push failed! rc=" + int_2_str(rc);
		 memcached_server_list_free(server);
		 return false;
    }
	memcached_server_list_free(server);

	//set
	rc = memcached_set(memc, key.c_str(), key.size(), value.c_str(), value.size(), expire_ts, 0);
    if(rc != MEMCACHED_SUCCESS)
    {
  		strErrMsg = "memcached_set failed! rc=" + int_2_str(rc);
		memcached_free(memc);
		return false;
    }

	memcached_free(memc);
	return true;	
}

std::string memcacheSessionGetValue(const std::string& ip, uint16_t port,
									const std::string& key, std::string& strErrMsg)
{
	if(key.empty())
	{
		strErrMsg = "empty key!!";
		return "";
	}

	//connect cache
	memcached_return rc;
	memcached_st *memc = memcached_create(NULL);
	memcached_server_st *server = memcached_server_list_append(NULL, ip.c_str(), port, &rc);
    rc = memcached_server_push(memc,server);
    if(MEMCACHED_SUCCESS != rc)
    {
         strErrMsg = "memcached_server_push failed! rc=" + int_2_str(rc);
		 memcached_server_list_free(server);
		 return "";
    }
	memcached_server_list_free(server);

	//get
	uint32_t dwFlags = 0;
    size_t szValueLength;
	char* cpValue = memcached_get(memc, key.c_str(), key.size(), &szValueLength, &dwFlags, &rc);
	if(MEMCACHED_NOTFOUND == rc)
    {
        strErrMsg = "memcached_get, session not found for key=" + key;
	    memcached_free(memc);
	    return "";
    }

    if(rc != MEMCACHED_SUCCESS)
    {
  		strErrMsg = "memcached_get failed! rc=" + int_2_str(rc);
		memcached_free(memc);
		return "";
    }

    string strResult(cpValue, szValueLength);
	
    memcached_free(memc);
    return strResult;
}



