#ifndef _HOOSHO_CGI_COMMON_JSAPI_SIGNATURE_H_
#define _HOOSHO_CGI_COMMON_JSAPI_SIGNATURE_H_

#include "cgi/cgi.h"
#include "../cgi_common_util.h"
#include "proto_io_tcp_client.h"
#include <sstream>
#include <stdio.h>

class CgiGetGoodInfoList: public lce::cgi::Cgi
{
public:
	 CgiGetGoodInfoList(uint32_t qwFlag, const std::string& strCgiConfigFile, const std::string& strLogConfigFile)
	 	:Cgi(qwFlag, strCgiConfigFile, strLogConfigFile)
	 {
	 	
	 }

public:
	virtual bool DerivedInit()
	{
		GetConfig().GetValue("DB", "db_ip", m_db_ip, "");
		GetConfig().GetValue("DB", "db_user", m_db_user, "");
		GetConfig().GetValue("DB", "db_passwd", m_db_passwd, "");
		GetConfig().GetValue("DB", "db_name", m_db_name, "");
		GetConfig().GetValue("DB", "table_name_good_info", m_table_name_good_info, "");
		GetConfig().GetValue("DB", "table_name_feed_info", m_table_name_feed_info, "");

		if(!mysql.Init(m_db_ip, m_db_name, m_db_user, m_db_passwd))
		{
			LOG4CPLUS_ERROR(logger, "msyql init failed");
			return false;
		}
		return true;
	}

	void GetGoodInfoByGoodId(const set<uint64_t> &setGoodid, vector<hoosho::msg::s::GoodInfo> &vecGoodInfo)
	{
		ostringstream oss;
		for(set<uint64_t>::const_iterator it = setGoodid.begin(); it != setGoodid.end(); ++it)
		{
			oss.str("");
			oss << "SELECT * FROM " << m_table_name_good_info 
				<< " WHERE good_id = " << (*it);
			LOG4CPLUS_DEBUG(logger, "sql=" << oss.str());
			SQL_QUERY_FAIL_CONTINUE(mysql, oss, logger);

			if(mysql.GetRowCount() && mysql.Next())
			{
				hoosho::msg::s::GoodInfo stGoodInfo;
				GoodInfoDB2PB(mysql, stGoodInfo);
				vecGoodInfo.push_back(stGoodInfo);
			}
			else
			{
				LOG4CPLUS_ERROR(logger, "good_id=" << (*it) << " not found");
			}
		}
	}
	
	virtual bool Process()
	{
		uint32_t dwReqType = (uint32_t) GetInput().GetValue("req_type");
		uint32_t dwReqNum = (uint32_t) GetInput().GetValue("req_num");

		ostringstream oss;
		set<uint64_t> setGoodid;
		vector<hoosho::msg::s::GoodInfo> vecGoodInfo;

		if(dwReqNum == 0 || dwReqNum > 10)
		{
			dwReqNum = 10;
		}

		if(dwReqType == 1)	//新品上市
		{
			oss.str("");
		}
		else if(dwReqType == 2)		//热门商品
		{
			oss.str("");
			oss << "SELECT *, COUNT(DISTINCT good_id) FROM " << m_table_name_feed_info
				<< " GROUP BY good_id ORDER BY current_join_num DESC LIMIT " << dwReqNum; 
			SQL_QUERY_FAIL_REPLY(mysql, oss, logger);
			while(mysql.GetRowCount() && mysql.Next())
			{
				hoosho::msg::s::FeedInfo stFeedInfo;
				FeedInfoDB2PB(mysql, stFeedInfo);
				GetAnyValue()["feed_info_list"].push_back(FeedInfoPB2Any(stFeedInfo));

				setGoodid.insert(stFeedInfo.good_id());
			}
		}
		else
		{
			LOG4CPLUS_ERROR(logger, "invalid req_type=" << dwReqType);
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			return true;
		}

		GetGoodInfoByGoodId(setGoodid, vecGoodInfo);
		for(size_t i = 0; i < vecGoodInfo.size(); i++)
		{
			GetAnyValue()["good_info_list"][int_2_str(vecGoodInfo[i].good_id())] = 
				GoodInfoPB2Any(vecGoodInfo[i]);
		}

		
		DoReply(CGI_RET_CODE_OK);
		return true;
	}

protected:
	std::string m_db_ip;
	std::string m_db_user;
	std::string m_db_passwd;
	std::string m_db_name;
	std::string m_table_name_good_info;
	std::string m_table_name_feed_info;
	lce::cgi::CMysql mysql;

protected:
	DECL_LOGGER(logger);
};

IMPL_LOGGER(CgiGetGoodInfoList, logger);

int main(int argc, char** argv)
{
	CgiGetGoodInfoList cgi(lce::cgi::Cgi::FLAG_POST_ONLY, "config.ini", "logger.properties");
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}

#endif

