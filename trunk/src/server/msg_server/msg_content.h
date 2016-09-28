#ifndef _MSG_SERVER_MSG_CONTENT_H_
#define _MSG_SERVER_MSG_CONTENT_H_

#include <string>
#include <vector>
#include "cgi/cgi.h"
#include "msg.pb.h"

typedef std::vector<hoosho::commstruct::MsgContent>& reVecMsgContent;

class MsgContent
{
public:
	MsgContent():m_id(0), m_create_ts(0), m_content(""), m_openid_md5_from(0), m_openid_md5_to(0), m_amount(0){}
	MsgContent(uint64_t id, uint64_t create_ts, std::string content, uint64_t openid_md5_from, uint64_t openid_md5_to)
				:m_id(id), m_create_ts(create_ts), m_content(content), m_openid_md5_from(openid_md5_from), m_openid_md5_to(openid_md5_to)
	{
	}

	int SelectFromDB(reVecMsgContent replyVec, lce::cgi::CMysql& mysql, std::string& strErrMsg);
    int DeleteFromDB(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int AddToDB(lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int GetMsgById(reVecMsgContent replyVec, lce::cgi::CMysql& mysql, std::string& strErrMsg);
	int GetMsgList(reVecMsgContent replyVec, lce::cgi::CMysql& mysql, std::string& strErrMsg);

	std::string ToString() const;

public:
	uint64_t m_id;
	uint64_t m_create_ts;
	std::string m_content;
	uint64_t m_openid_md5_from;
	uint64_t m_openid_md5_to;
	uint16_t m_amount;
} ;


#endif

