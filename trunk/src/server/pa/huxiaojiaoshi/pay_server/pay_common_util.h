#ifndef _PAY_COMMON_UTIL_H_
#define _PAY_COMMON_UTIL_H_

#include <string>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <algorithm>
#include <time.h>
#include "util/lce_util.h"
#include <map>

typedef struct
{
	uint64_t total_income;
	uint64_t total_outcome;
	uint64_t total_outcome_pending;
}UserCash;

#define int_2_str(i) lce::util::StringOP::TypeToStr(i) 

std::string str_replace(std::string& srcstr, const std::string& substr, const std::string& repstr);
std::string XML_ESCAPE(std::string& srcstr);
std::string genSignString(std::map<std::string, std::string>& mapParams, const std::string& key);
std::string genRandomString(int length);
std::string genLocalTimeString(int expire_s);
std::string genOutTradeNoString();
std::string genRedPackBillNoString(const std::string& mch_id);

bool memcacheSessionAdd(const std::string& ip, uint16_t port, 
								const std::string& key, const std::string& value, 
								uint32_t expire_ts, std::string& strErrMsg);

std::string memcacheSessionGetValue(const std::string& ip, uint16_t port,
									const std::string& key, std::string& strErrMsg);



#endif
