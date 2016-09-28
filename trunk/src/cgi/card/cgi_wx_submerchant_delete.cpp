#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWxSubmerchantDelete: public HooshoCgiCard
{
public:
	CgiWxSubmerchantDelete() :
			HooshoCgiCard(FLAG_POST_ONLY, "config_card.ini", "logger.properties_card", true)
	{

	}

	bool InnerProcess()
	{
		string strMerchantid = (string) GetInput().GetValue("merchant_id");

		EMPTY_STR_RETURN(strMerchantid);
		string strErrMsg = "";
		SubmerchantInfo stSubmerchantInfo(m_table_name_submerchant_info);
		stSubmerchantInfo.m_id = strMerchantid;
		CHECK_DB(stSubmerchantInfo,strErrMsg);
		if(stSubmerchantInfo.m_uin != m_user_info.m_uin)
		{
			LOG4CPLUS_ERROR(logger, "merchant_id = " << strMerchantid << ",  stSubmerchantInfo.SelectFromDB failed, errmsg = " << strErrMsg);
			DoReply(CGI_RET_CODE_NO_PREVILEDGES);
			return true;
		}
		stSubmerchantInfo.m_status = SUBMERCHANT_DELETE;
		if(stSubmerchantInfo.UpdateToDB(strErrMsg) != TableBaseCard::TABLE_BASE_RET_OK)
		{
			LOG4CPLUS_ERROR(logger, "merchant_id = " << strMerchantid << ",  stSubmerchantInfo.UpdateToDB failed, errmsg = " << strErrMsg);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
	
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiWxSubmerchantDelete cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}
