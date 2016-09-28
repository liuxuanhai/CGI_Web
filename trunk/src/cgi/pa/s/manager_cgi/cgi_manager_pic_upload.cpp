#include "cgi_manager_base.h"
#include "common_util.h"

class CgiManagerPicUpload:public CgiManagerBase
{
public:
	CgiManagerPicUpload():CgiManagerBase(0, "config.ini", "logger.properties", true)
	{

	}

	bool InnerProcess()
	{

		string strPicData = (string) GetInput().GetFileData("pic_data");
		if(strPicData.empty())
		{
			DoReply(CGI_RET_CODE_INVALID_PARAM);
			LOG4CPLUS_ERROR(logger, "empty pic data");
			return true;
		}

		LOG4CPLUS_DEBUG(logger, "pic.size=" << strPicData.size());

		std::ostringstream oss;

		uint64_t qwPicId = ::common::util::generate_unique_id();
		oss.str("");
		oss << "INSERT INTO " << m_table_name_pic_info
			<< " SET pic_id = " << qwPicId
			<< ", content = '" << sql_escape(strPicData) << "'"
			<< ", user_id = " << atoi(m_login_phone.c_str())
			<< ", type = " << hoosho::msg::s::PIC_TYPE_PUBLIC;
		SQL_QUERY_FAIL_REPLY(m_mysql_helper, oss, logger);

		LOG4CPLUS_DEBUG(logger, "pic_id=" << qwPicId);
		GetAnyValue()["pic_id"] = int_2_str(qwPicId);
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiManagerPicUpload cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
