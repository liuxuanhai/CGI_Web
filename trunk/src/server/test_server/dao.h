#ifndef _TEST_SERVER_DAO_H_
#define _TEST_SERVER_DAO_H_

#include <string>
#include <vector>
#include "cgi/cgi.h"

typedef struct DaoUserInfo
{
	uint32_t m_id;
	std::string m_name;
	uint32_t m_age;

	DaoUserInfo():m_id(0), m_name(""), m_age(0){}
	DaoUserInfo(uint32_t id, const std::string& name, uint32_t age)
				:m_id(id), m_name(name), m_age(age)
	{
	}

	int SelectFromDB(lce::cgi::CMysql& mysql, std::string& strErrMsg);
    int UpdateToDB(lce::cgi::CMysql& mysql, std::string& strErrMsg);
    int DeleteFromDB(lce::cgi::CMysql& mysql, std::string& strErrMsg);

	std::string ToString() const;
} DaoUserInfo;


#endif

