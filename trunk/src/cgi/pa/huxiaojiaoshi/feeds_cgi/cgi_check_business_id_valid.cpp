#include "cgi_feeds_base.h"

class CgiCheckListen: public CgiFeedsBase
{
public:
	CgiCheckListen():
		CgiFeedsBase(0, "config.ini", "logger.properties", true)
	{

	}

	bool InnerProcess()
	{

		//get HTTP params
		uint64_t qwBusinessId = strtoul(((string)GetInput().GetValue("business_id")).c_str(), NULL, 10);
		uint64_t qwBusinessType = strtoul(((string)GetInput().GetValue("business_type")).c_str(), NULL, 10);

		ZERO_INT_RETURN(qwBusinessId);
		ZERO_INT_RETURN(qwBusinessType);

		uint64_t qwErrcode = 0;
		std::string strErrMsg = "";

		if(CheckBusinessIdValid(m_feeds_server_ip, m_feeds_server_port, qwBusinessId, qwBusinessType,
							  qwErrcode, strErrMsg) < 0)
		{
			LOG4CPLUS_ERROR(logger, strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		SERVER_NOT_OK_RETURN(qwErrcode);

		DoReply(CGI_RET_CODE_OK);
		return true;		
	}

};

int main()
{
	CgiCheckListen cgi;
	if(!cgi.Run())
	{
		return -1;
	}
	return 0;
}
