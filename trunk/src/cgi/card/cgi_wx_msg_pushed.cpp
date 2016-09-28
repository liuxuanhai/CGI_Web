#include "hoosho_cgi_card.h"
#include "wx_https_req.h"

class CgiWXMsgPushed: public HooshoCgiCard
{
public:
	CgiWXMsgPushed() :
			HooshoCgiCard(0, "config_card.ini", "logger.properties_card", NO_LOGIN)
	{

	}

	bool InnerProcess()
	{			
	
		int iRet = 0;
		std::string strErrMsg = "";
		std::string strSignature = "";
		std::string strTimeStamp = "";
		std::string strNonce = "";
		std::string strEchostr = "";
		std::string strMsgSignature = "";
		string strQueryString = (string) GetInput().GetQueryString();
		vector<string> vecQueryString;
		lce::cgi::Split(strQueryString, "&", vecQueryString);
		for (size_t i = 0; i != vecQueryString.size(); ++i)
		{
			vector<string> vecParams;
			lce::cgi::Split(vecQueryString[i], "=", vecParams);
			if (vecParams.size() != 2)
			{
				continue;
			}

			string strKey = vecParams[0];
			string strValue = vecParams[1];
			if (strKey == "signature")
				strSignature = strValue;
			if (strKey == "nonce")
				strNonce = strValue;
			if (strKey == "timestamp")
				strTimeStamp = strValue;
			if (strKey == "echostr")
				strEchostr = strValue;
			if (strKey == "msg_signature")
				strMsgSignature = strValue;
		}

		LOG4CPLUS_DEBUG(logger, "strMsgSignature="<<strSignature <<", strTimeStamp="<<strTimeStamp <<", strNonce="<<strNonce);
		string strSign = "";
		WXHttpsReq::CheckSign(strNonce, strTimeStamp, strSign);
		LOG4CPLUS_DEBUG(logger, "strMakeSign="<<strSign<<", strWXSign="<<strSignature);
		if (strSignature != strSign)
		{
			LOG4CPLUS_DEBUG(logger, "strMakeSign="<<strSign<<", strWXSign="<<strSignature<<",not match");
			SetOutputJson("error");
			return false;

		}
		string strPostData = GetInput().GetPostData();

		if (strPostData == "" && strEchostr != "") //服务器验证
		{
			LOG4CPLUS_DEBUG(logger, "strEchostr="<<strEchostr);
			SetOutputJson(strEchostr);
			DoReply(CGI_RET_CODE_OK);
			return true;

		}


		std::map<std::string, std::string> mapMsg;
		iRet = WXHttpsReq::WXMsgDecrypt(strMsgSignature, strTimeStamp, strNonce, strPostData, WX_HX_PLATFORM_DEV_MSG_CHECK_TOKEN, WX_HX_PLATFORM_DEV_MSG_CRYPT_KEY, WX_HX_PLATFORM_DEV_APPID, mapMsg);
		if (iRet < 0)
		{
			LOG4CPLUS_ERROR(logger, "WXMsgDecrypt failed");
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}
		//deal msg
		string strMsgType = mapMsg["MsgType"];
		if (strMsgType == "")
		{
			LOG4CPLUS_ERROR(logger, "invalid wx msg_type="<<strMsgType);
			DoReply(CGI_RET_CODE_SERVER_BUSY);
			return true;
		}

		string strNow = int_2_str(time(0));
		static char aczMsgBuffer[2048];

		// event msg
		if (strMsgType == "event")
		{

			string strEvent = mapMsg["Event"];
			string strToUserName = mapMsg["ToUserName"];
			string strFromUserName = mapMsg["FromUserName"];
			string strCreateTime = mapMsg["CreateTime"];

			LOG4CPLUS_DEBUG(logger, "1.1 event msg, event="<<strEvent<<", strToUserName="<<strToUserName<<", strFromUserName="<<strFromUserName <<", strCreateTime="<<strCreateTime);

			if (strEvent == "card_pass_check")
			{
				LOG4CPLUS_DEBUG(logger, "member card check success, cardid="<<mapMsg["CardId"]);

				CardInfo stCardInfo(m_table_name_card_info);
				stCardInfo.m_id = mapMsg["CardId"];
				CHECK_DB(stCardInfo,strErrMsg);

				if (1 == stCardInfo.m_status)
				{
					LOG4CPLUS_ERROR(logger, "status=1, already check!!!!! fuck , get out");
					DoReply(CGI_RET_CODE_SERVER_BUSY);
					return true;
				}

				stCardInfo.m_status = CARD_PASS;

				UPDATE_DB(stCardInfo,strErrMsg);
				LOG4CPLUS_DEBUG(logger, "change card_info status  to 1, succ!!");
			}
			else if (strEvent == "card_not_pass_check")
			{
				LOG4CPLUS_DEBUG(logger, "member card check failed, cardid="<<mapMsg["CardId"]);
				CardInfo stCardInfo(m_table_name_card_info);
				stCardInfo.m_id = mapMsg["CardId"];
				CHECK_DB(stCardInfo,strErrMsg);

				stCardInfo.m_status = CARD_NOT_PASS;

				UPDATE_DB(stCardInfo,strErrMsg);
				LOG4CPLUS_DEBUG(logger, "change card_info status  to 2, succ!!");
			}
			else if (strEvent == "user_pay_from_pay_cell")
			{
				string strCardid = mapMsg["CardId"];
				string strUserCardCode = mapMsg["UserCardCode"];
				string strTransId = mapMsg["TransId"];
				string strLocationId = mapMsg["LocationId"];
				string strFee = mapMsg["Fee"];
				string strOriginalFee = mapMsg["OriginalFee"];
				LOG4CPLUS_DEBUG(logger, "CardId= "<<strCardid<<", UserCardCode= "<<strUserCardCode<<", TransId= "<<strTransId<<", LocationId= "<<strLocationId<<", Fee= "<<strFee<<", OriginalFee= "<<strOriginalFee);
			}
			else if (strEvent == "user_del_card")
			{
				string strCardid = mapMsg["CardId"];
				string strUserCardCode = mapMsg["UserCardCode"];
				LOG4CPLUS_DEBUG(logger, "CardId= "<<strCardid<<", UserCardCode= "<<strUserCardCode);
			}
			else if (strEvent == "user_consume_card")
			{
				string strCardid = mapMsg["CardId"];
				string strUserCardCode = mapMsg["UserCardCode"];
				string strConsumeSource = mapMsg["ConsumeSource"];
				string strLocationName = mapMsg["LocationName"];
				string strStaffOpenId = mapMsg["StaffOpenId"];
				LOG4CPLUS_DEBUG(logger, "CardId= "<<strCardid<<", UserCardCode= "<<strUserCardCode<<", ConsumeSource= "<<strConsumeSource<<", LocationName= "<<strLocationName<<", StaffOpenId= "<<strStaffOpenId);
			}
			else if (strEvent == "user_get_card")
			{
				string strCardid = mapMsg["CardId"];
				string strIsGiveByFriend = mapMsg["IsGiveByFriend"];
				string strUserCardCode = mapMsg["UserCardCode"];
				string strFriendUserName = mapMsg["FriendUserName"];
				string strOuterId = mapMsg["OuterId"];
				string strOldUserCardCode = mapMsg["OldUserCardCode"];
				LOG4CPLUS_DEBUG(logger, "CardId= "<<strCardid<<", IsGiveByFriend= "<<strIsGiveByFriend<<", UserCardCode= "<<strUserCardCode<<", FriendUserName= "<<strFriendUserName<<", OuterId= "<<strOuterId<<", OldUserCardCode= "<<strOldUserCardCode);
			}
			else if (strEvent == "user_view_card")
			{
				string strCardid = mapMsg["CardId"];
				string strUserCardCode = mapMsg["UserCardCode"];
				LOG4CPLUS_DEBUG(logger, "CardId= "<<strCardid<<", UserCardCode= "<<strUserCardCode);
			}
			else if (strEvent == "user_enter_session_from_card")
			{
				string strCardid = mapMsg["CardId"];
				string strUserCardCode = mapMsg["UserCardCode"];
				LOG4CPLUS_DEBUG(logger, "CardId= "<<strCardid<<", UserCardCode= "<<strUserCardCode);
			}
			else if (strEvent == "submit_membercard_user_info")
			{
				string strCardid = mapMsg["CardId"];
				string strUserCardCode = mapMsg["UserCardCode"];
				LOG4CPLUS_DEBUG(logger, "CardId= "<<strCardid<<", UserCardCode= "<<strUserCardCode);
			}
			else if (strEvent == "MASSSENDJOBFINISH")
			{
				string strMsgID = mapMsg["MsgID"];
				string strStatus = mapMsg["Status"];
				// string strTotalCount = mapMsg["TotalCount"];
				// string strFilterCount = mapMsg["FilterCount"];
				// string strSentCount = mapMsg["SentCount"];
				// string strErrorCount = mapMsg["ErrorCount"];
				LOG4CPLUS_DEBUG(logger, "MsgID= "<<strMsgID<<", Status= "<<strStatus);
				// LOG4CPLUS_DEBUG(logger, "TotalCount= "<<strTotalCount<<", FilterCount= "<<strFilterCount);
				// LOG4CPLUS_DEBUG(logger, "SentCount= "<<strSentCount<<", ErrorCount= "<<strErrorCount);
			}
			else if (strEvent == "poi_check_notify")
			{
				// get sid first  where UniqId is the same as sid
				std::string strUniqId = mapMsg["UniqId"];
				if (strUniqId.empty())
				{
					LOG4CPLUS_ERROR(logger, "wx call back req invalid!!");
					DoReply(CGI_RET_CODE_SERVER_BUSY);
					return true;
				}

				EntityShopInfo stEntityShopInfo(m_table_name_entity_shop_info);
				stEntityShopInfo.m_sid = strUniqId;
				// check success or fail
				CHECK_DB(stEntityShopInfo, strErrMsg);
				std::string strResult = mapMsg["result"];
				LOG4CPLUS_DEBUG(logger, "UniqId= "<<strUniqId << ",result="<<strResult);
				if (strResult.empty())
				{
					LOG4CPLUS_ERROR(logger, "wx call back strResult invalid!!");
					DoReply(CGI_RET_CODE_SERVER_BUSY);
					return true;
				}

				if (strResult == "succ")
				{
					std::string strPoiId = mapMsg["PoiId"];
					LOG4CPLUS_DEBUG(logger, "strPoiId= "<<strPoiId);
					if (strPoiId.empty() || strCreateTime.empty())
					{
						LOG4CPLUS_ERROR(logger, "wx call back strPoiId or strCreateTime invalid!!");
						DoReply(CGI_RET_CODE_SERVER_BUSY);
						return true;
					}
					uint64_t intCreateTime = std::atoi(strCreateTime.c_str());
					stEntityShopInfo.m_PoiId = strPoiId;
					stEntityShopInfo.m_CreateTime = intCreateTime;
					stEntityShopInfo.m_available_state = ENTITYSHOP_PASS;
					stEntityShopInfo.m_update_status = ENTITY_SHOP_STATUS_NO_UPDATE; // new == no update
				}
				else if (strResult == "fail")
				{
					std::string strMsg = mapMsg["msg"];
					LOG4CPLUS_DEBUG(logger, "msg= "<<strMsg);
					if (strMsg.empty())
					{
						LOG4CPLUS_ERROR(logger, "wx call back strMsg invalid!!");
						DoReply(CGI_RET_CODE_SERVER_BUSY);
						return true;
					}
			//		stEntityShopInfo.m_PoiId = "-1"; // "-1 indicates fail, 0 indicates waiting"
					stEntityShopInfo.m_available_state = ENTITYSHOP_NOT_PASS;
					stEntityShopInfo.m_err_msg = strMsg;
				}
				else
				{
					LOG4CPLUS_ERROR(logger, "wx call back strResult(!succ && !fail) invalid!!");
					DoReply(CGI_RET_CODE_SERVER_BUSY);
					return true;
				}

				// update db
				UPDATE_DB(stEntityShopInfo,strErrMsg);

				LOG4CPLUS_DEBUG(logger, "entity shop result done  --- updated db !");

			}
			else if(strEvent == "card_merchant_check_result")
			{
				std::string strMerchantId = mapMsg["MerchantId"];
				std::string strIsPass = mapMsg["IsPass"];
				std::string strReason = mapMsg["Reason"];
				SubmerchantInfo stSubmerchantInfo(m_table_name_submerchant_info);
				stSubmerchantInfo.m_id = strMerchantId;
//				std::string strErrMsg;
				CHECK_DB(stSubmerchantInfo,strErrMsg);
				if(strIsPass == "1")
				{
					stSubmerchantInfo.m_status = SUBMERCHANT_PASS;
					stSubmerchantInfo.m_wx_msg = "";
				}
				else
				{
					stSubmerchantInfo.m_status = SUBMERCHANT_NOT_PASS;
					stSubmerchantInfo.m_wx_msg = strReason;
				}
				UPDATE_DB(stSubmerchantInfo,strErrMsg);

			}
			else
			{
				// bzero(aczMsgBuffer, 2048);
				// snprintf(aczMsgBuffer, 2048,
				// 		"<xml><ToUserName><![CDATA[%s]]></ToUserName>"
				// 				"<FromUserName><![CDATA[%s]]></FromUserName>"
				// 				"<CreateTime>%s</CreateTime>"
				// 				"<MsgType><![CDATA[text]]></MsgType>"
				// 				"<Content><![CDATA[%s]]></Content></xml>",
				// 		strFromUserName.c_str(), strToUserName.c_str(),
				// 		strNow.c_str(), (strEvent + "from_callback").c_str());

				// string strEncryptMsg = "";
				// iRet = WXHttpsReq::WXMsgEncrypt(aczMsgBuffer, strNow, strNonce,
				// 		WX_THIRD_PLATFORM_DEV_MSG_CHECK_TOKEN,
				// 		WX_THIRD_PLATFORM_DEV_MSG_CRYPT_KEY,
				// 		WX_THIRD_PLATFORM_DEV_APPID, strEncryptMsg);
				// if (iRet < 0) {
				// 	LOG4CPLUS_ERROR(logger,
				// 			"WXHttpsReq::WXMsgEncrypt failed!!");
				// 	DoReply(CGI_RET_CODE_SERVER_BUSY);
				// 	return true;
				// }

				// SetOutputJson(strEncryptMsg);
			}

			SetOutputJson("");
			DoReply(CGI_RET_CODE_OK);
			return true;

		}

		//text msg
		if (strMsgType == "text")
		{
			string strToUserName = mapMsg["ToUserName"];
			string strFromUserName = mapMsg["FromUserName"];
			string strCreateTime = mapMsg["CreateTime"];
			string strContent = mapMsg["Content"];
			string strMsgId = mapMsg["MsgId"];

			LOG4CPLUS_DEBUG(logger, "strToUserName="<<strToUserName <<", strFromUserName="<<strFromUserName <<", strCreateTime="<<strCreateTime <<", strContent="<<strContent <<", strMsgId="<<strMsgId);

			if (strContent == "secret")
			{
				bzero(aczMsgBuffer, 2048);
				snprintf(aczMsgBuffer, 2048, "<xml><ToUserName><![CDATA[%s]]></ToUserName>"
						"<FromUserName><![CDATA[%s]]></FromUserName>"
						"<CreateTime>%s</CreateTime>"
						"<MsgType><![CDATA[text]]></MsgType>"
						"<Content><![CDATA[%s]]></Content></xml>", strFromUserName.c_str(), strToUserName.c_str(), strNow.c_str(), "良晨美景，不可或缺。");

				string strEncryptMsg = "";
				iRet = WXHttpsReq::WXMsgEncrypt(aczMsgBuffer, strNow, strNonce, WX_HX_PLATFORM_DEV_MSG_CHECK_TOKEN, WX_HX_PLATFORM_DEV_MSG_CRYPT_KEY, WX_HX_PLATFORM_DEV_APPID, strEncryptMsg);
				if (iRet < 0)
				{
					LOG4CPLUS_ERROR(logger, "WXHttpsReq::WXMsgEncrypt failed!!");
					DoReply(CGI_RET_CODE_SERVER_BUSY);
					return true;
				}

				SetOutputJson(strEncryptMsg);
				DoReply(CGI_RET_CODE_OK);
				return true;
			}
			else
			{
				bzero(aczMsgBuffer, 2048);
				snprintf(aczMsgBuffer, 2048, "<xml><ToUserName><![CDATA[%s]]></ToUserName>"
						"<FromUserName><![CDATA[%s]]></FromUserName>"
						"<CreateTime>%s</CreateTime>"
						"<MsgType><![CDATA[text]]></MsgType>"
						"<Content><![CDATA[%s]]></Content></xml>", strFromUserName.c_str(), strToUserName.c_str(), strNow.c_str(), "请按套路出牌");

				string strEncryptMsg = "";
				iRet = WXHttpsReq::WXMsgEncrypt(aczMsgBuffer, strNow, strNonce, WX_HX_PLATFORM_DEV_MSG_CHECK_TOKEN, WX_HX_PLATFORM_DEV_MSG_CRYPT_KEY, WX_HX_PLATFORM_DEV_APPID, strEncryptMsg);
				if (iRet < 0)
				{
					LOG4CPLUS_ERROR(logger, "WXHttpsReq::WXMsgEncrypt failed!!");
					DoReply(CGI_RET_CODE_SERVER_BUSY);
					return true;
				}

				SetOutputJson(strEncryptMsg);
				DoReply(CGI_RET_CODE_OK);
				return true;
			}

		}
		SetOutputJson("");
		DoReply(CGI_RET_CODE_OK);
		return true;
	}
};

int main()
{
	CgiWXMsgPushed cgi;
	if (!cgi.Run())
	{
		return -1;
	}
	return 0;
}

