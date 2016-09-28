#ifndef _COMMON_UTIL_H_
#define _COMMON_UTIL_H_

#include <stdint.h>
#include <string>

namespace common
{
	namespace util
	{
		uint64_t generate_unique_id();
		uint64_t generate_session_id(uint64_t md5_a, uint64_t md5_b);
		uint64_t generate_session_id_bystr(const std::string& str_a, const std::string& str_b);
		uint64_t charToUint64_t(const char* src);
		uint32_t GetRandNum(uint32_t dwFrom, uint32_t dwTo);
		uint64_t GetCurrentMs();
	}
}

#endif

