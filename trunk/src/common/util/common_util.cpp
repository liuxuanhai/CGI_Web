#include "common_util.h"
#include <sys/time.h>
#include <stdlib.h>
#include <lce/util/md5.h>
#include <sstream>

namespace common
{
	namespace util
	{
		uint64_t generate_unique_id()
		{
			timeval tv;
			gettimeofday(&tv, NULL);
			uint64_t qwMS = (uint64_t)tv.tv_sec * 1000;
			qwMS += ((uint64_t)tv.tv_usec / 1000);
			srand(tv.tv_usec ^ 77);
			uint16_t randNum = rand() % 65536;

			//高6字节为毫秒， 低2字节为随机数
			uint64_t qwRetId = (qwMS << 16) | randNum;
			return qwRetId;	
		}
		
		uint64_t generate_session_id(uint64_t md5_a, uint64_t md5_b)
		{
			uint64_t max = md5_a > md5_b ? md5_a : md5_b;
			uint64_t min = md5_a < md5_b ? md5_a : md5_b;

			std::ostringstream oss;
			oss.str("");
			oss<<max<<min;

			md5 m_md5;
			return m_md5.hash64(oss.str().c_str(),strlen(oss.str().c_str()));			
		}

		uint64_t generate_session_id_bystr(const std::string& str_a, const std::string& str_b)
		{
			std::string max = str_a > str_b ? str_a : str_b;
			std::string min = str_a < str_b ? str_a : str_b;

			std::ostringstream oss;
			oss.str("");
			oss<<max<<min;

			md5 m_md5;
			return m_md5.hash64(oss.str().c_str(),strlen(oss.str().c_str()));	
		}

		uint64_t charToUint64_t(const char* src)
		{
			uint64_t ret;
			std::stringstream sst;
			sst.str("");
			sst<<src;
			sst>>ret;

			return ret;
		}

		static uint32_t __GetRandNum__(uint32_t dwInitSeed)
		{
		    if (0 == dwInitSeed)
		    {    
		        timeval tv;
		        gettimeofday(&tv, NULL);
		        srand(tv.tv_usec ^ 77); 
		    }    
		    else 
		    {    
		        srand(dwInitSeed ^ 77); 
		    }    

		    return rand();
		}

		uint32_t GetRandNum(uint32_t dwFrom, uint32_t dwTo)
		{
		    if (dwFrom > dwTo)
		    {    
		        uint32_t dwTmp = dwFrom;
		        dwFrom = dwTo;
		        dwTo = dwTmp;
		    }    

		    uint32_t r = __GetRandNum__(0);
		    return dwFrom + (r % (dwTo - dwFrom + 1)); 
		}

		uint64_t GetCurrentMs()
		{
			timeval tv;
			gettimeofday(&tv, NULL);
			uint64_t qwMS = (uint64_t)tv.tv_sec * 1000;
			qwMS += ((uint64_t)tv.tv_usec / 1000);

			return qwMS;
		}
	}
}
