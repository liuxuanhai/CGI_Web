#include "main_cgi.h"

class CgiOperOrderAdd: public MainCgi
{
     public:
		CgiOperOrderAdd() : MainCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
        {

        }

		bool InnerProcess()
		{
			if(m_user_info.m_user_type != USER_TYPE_DEMAND_SIDE)
			{
				DoReply(CGI_RET_CODE_NO_PREVILEDGES);
				LOG4CPLUS_ERROR(logger, "add dev order failed, not demand side user, uin="<<m_user_info.m_uin);
				return true;
			}

			uint64_t qwTag =(uint64_t)GetInput().GetValue("tag");
			std::string strWXList =(string)GetInput().GetValue("wx_list");
			std::string strSpreadName =(string)GetInput().GetValue("spread_name");
			uint32_t dwSpreadType =(uint32_t)GetInput().GetValue("spread_type");
			uint32_t dwSpreadPos =(uint32_t)GetInput().GetValue("spread_pos");
			uint64_t qwSpreadStartTs =(uint64_t)GetInput().GetValue("spread_start_ts");
			std::string strSpreadCertifiedPicUrl = (string)GetInput().GetValue("spread_certified_pic_url");
			std::string strSpreadRemark = (string)GetInput().GetValue("spread_ramark");
			std::string strChapterTitle = (string)GetInput().GetValue("chapter_title");
			std::string strChapterAuthor = (string)GetInput().GetValue("chapter_author");
			std::string strChapterCoverUrl = (string)GetInput().GetValue("chapter_cover_url");
			uint32_t dwCoverInsertMainBody = (uint32_t)GetInput().GetValue("chapter_cover_insert_main_body");
			std::string strChapterSummary = (string)GetInput().GetValue("chapter_summary");
			std::string strChapterMainBody = (string)GetInput().GetValue("chapter_main_body");
			std::string strChapterOrginalUrl = (string)GetInput().GetValue("chapter_original_url");
			ZERO_INT_RETURN(qwTag);
			EMPTY_STR_RETURN(strWXList);
			EMPTY_STR_RETURN(strSpreadName);
			if(dwSpreadType != SPREAD_TYPE_SOFT && dwSpreadType != SPREAD_TYPE_HARD)
			{
				DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "invalid param dwSpreadType="<<dwSpreadType);
				return true;
			}

			if(dwSpreadPos > SPREAD_POS_SINGLE_PIC_TEXT || dwSpreadPos < SPREAD_POS_MULTI_PIC_TEXT_FIRST)
			{
				DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "invalid param dwSpreadPos="<<dwSpreadPos);
				return true;
			}
			if(qwSpreadStartTs < (uint64_t)(time(0) + 24 * 3600))
			{
				//一天后才能开始推广， 至少了
				DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "invalid param qwSpreadStartTs="<<qwSpreadStartTs<<", too early !");
				return true;
			}

			EMPTY_STR_RETURN(strChapterTitle);
			EMPTY_STR_RETURN(strChapterAuthor);
			EMPTY_STR_RETURN(strChapterCoverUrl);
			EMPTY_STR_RETURN(strChapterMainBody);

			int iRet = 0;
			std::string strErrMsg = "";
			
			//check wxpa valid
			std::map<std::string, std::string> mapWx2OperId;
			std::vector<std::string> vecWx;
			lce::cgi::Split(strWXList, "__STRUCTSEP__", vecWx);
			if(vecWx.empty())
			{
				DoReply(CGI_RET_CODE_INVALID_PARAM);
				LOG4CPLUS_ERROR(logger, "invalid param wx_list="<<strWXList);
				return true;
			}
			
			for(size_t i=0; i!=vecWx.size(); ++i)
			{
				WXPublicAccount stWXPublicAccount(m_table_name_wx_public_account);
				stWXPublicAccount.m_wx = vecWx[i];
				iRet = stWXPublicAccount.SelectFromDB(strErrMsg);
				if(iRet == TableBase::TABLE_BASE_RET_NOT_EXIST)
				{
					DoReply(CGI_RET_CODE_INVALID_PARAM);
					LOG4CPLUS_ERROR(logger, "not exists wx="<<stWXPublicAccount.m_wx);
					return true;
				}

				if(iRet != TableBase::TABLE_BASE_RET_OK)
				{
					DoReply(CGI_RET_CODE_SERVER_BUSY);
					LOG4CPLUS_ERROR(logger, "stWXPublicAccount.SelectFromDB failed, errmsg="<<strErrMsg);
					return true;
				}
				
				if(stWXPublicAccount.m_status != WX_PUBLIC_ACCOUNT_VERIFY_OK)
				{
					DoReply(CGI_RET_CODE_INVALID_PARAM);
					LOG4CPLUS_ERROR(logger, "can not use, wxpa.status="<<stWXPublicAccount.m_status);
					return true;
				}

				mapWx2OperId[stWXPublicAccount.m_wx] = stWXPublicAccount.m_oper_id;
			}
			
			//add oper order
            OperOrder stOperOrder(m_table_name_oper_order);
			stOperOrder.m_id = GenerateTransID(ID_TYPE_ORDER_OPER);
			stOperOrder.m_need_uin = m_user_info.m_uin;
			stOperOrder.m_tag = qwTag;
			stOperOrder.m_wx_list = strWXList;
			stOperOrder.m_spread_name = strSpreadName;
			stOperOrder.m_spread_type = dwSpreadType;
			stOperOrder.m_spread_pos = dwSpreadPos;
			stOperOrder.m_spread_start_ts = qwSpreadStartTs;
			stOperOrder.m_spread_certified_pic_url = strSpreadCertifiedPicUrl;
			stOperOrder.m_spread_ramark = strSpreadRemark;
			stOperOrder.m_chapter_title = strChapterTitle;
			stOperOrder.m_chapter_author = strChapterAuthor;
			stOperOrder.m_chapter_cover_url = strChapterCoverUrl;
			stOperOrder.m_chapter_cover_insert_main_body = dwCoverInsertMainBody;
			stOperOrder.m_chapter_summary = strChapterSummary;
			stOperOrder.m_chapter_main_body = strChapterMainBody;
			stOperOrder.m_chapter_original_url = strChapterOrginalUrl;
			stOperOrder.m_status = OPER_ORDER_STATUS_VERIFY_PENDING;
			stOperOrder.m_create_ts = time(0);
			stOperOrder.m_cancel_ts = 0;
			if(stOperOrder.m_id.empty())
			{
				LOG4CPLUS_ERROR(logger, "generate oper_order id failed, fuck, add dev order failed");
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}
			
			if(stOperOrder.UpdateToDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
			{
				LOG4CPLUS_ERROR(logger, "stOperOrder.UpdateToDB failed, errmsg="<<strErrMsg);
				DoReply(CGI_RET_CODE_SERVER_BUSY);
				return true;
			}

			//add oper order reverse
			for(size_t i=0; i<vecWx.size(); ++i)
			{
				OperOrderReverse stOperOrderReverse(m_table_name_oper_order_reverse);
				stOperOrderReverse.m_wx = vecWx[i];
				stOperOrderReverse.m_oper_order_id =stOperOrder.m_id;
				stOperOrderReverse.m_oper_id = mapWx2OperId[stOperOrderReverse.m_wx];
				if(stOperOrderReverse.UpdateToDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
				{
					LOG4CPLUS_ERROR(logger, "stOperOrderReverse.UpdateToDB failed, errmsg="<<strErrMsg);
				}
			}

			//succ , return
			GetAnyValue()["oper_order_id"] = stOperOrder.m_id;
			DoReply(CGI_RET_CODE_OK);
			return true;
		}
};


int main()
{
	CgiOperOrderAdd cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}




