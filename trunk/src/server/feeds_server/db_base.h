#ifndef _FEEDS_SERVER_DB_BASE_H_
#define _FEEDS_SERVER_DB_BASE_H_

#include "util/lce_util.h"
#include "msg.pb.h"


class TableBase
{
public:
	TableBase(){}
	~TableBase(){}

	enum DB_RET
	{
			DB_RET_OK = 0,
			DB_RET_NOT_EXIST = 10001,
			DB_RET_FAIL = 10002,
	};
	
	string IntToHexStr(uint32_t num)
	{
	    char index[4];
		sprintf(index, "%02x", num);
		return string(index);
	}
};	

#define int_2_str(i) lce::util::StringOP::TypeToStr(i) 

#endif

