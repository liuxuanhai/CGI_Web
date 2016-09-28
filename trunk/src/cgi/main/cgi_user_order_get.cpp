#include "main_cgi.h"

class CgiUserOrderGet: public MainCgi
{
     public:
        CgiUserOrderGet() : MainCgi(FLAG_POST_ONLY, "config.ini", "logger.properties", CGI_NEED_LOGIN_YES)
        {

        }

		void DevOrderGet(std::vector<DevOrder>& stDevOrderList)
		{
			std::string strErrMsg = "";
			if(m_user_info.m_user_type == USER_TYPE_DEMAND_SIDE)
			{
				DevOrderWithNeedUin stOrderList(m_table_name_dev_order);
				stOrderList.m_need_uin = m_user_info.m_uin;
				if(stOrderList.SelectFromDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
				{
					LOG4CPLUS_ERROR(logger, "stOrderList.SelectFromDB failed, errmsg="<<strErrMsg);
				}
				else
				{
					stDevOrderList = stOrderList.m_dev_order_list;
				}
			}
			else if(!m_user_info.m_dev_id.empty())
			{
				DevOrderWithDevId stOrderList(m_table_name_dev_order);
				stOrderList.m_dev_id = m_user_info.m_dev_id;
				if(stOrderList.SelectFromDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
				{
					LOG4CPLUS_ERROR(logger, "stOrderList.SelectFromDB failed, errmsg="<<strErrMsg);
				}
				else
				{
					stDevOrderList = stOrderList.m_dev_order_list;
				}
			}

			return;
		}

		void OperOrderGet(std::vector<OperOrder>& stOperOrderList)
		{
			std::string strErrMsg = "";
			if(m_user_info.m_user_type == USER_TYPE_DEMAND_SIDE)
			{
				OperOrderWithNeedUin stOrderList(m_table_name_oper_order);
				stOrderList.m_need_uin = m_user_info.m_uin;
				if(stOrderList.SelectFromDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
				{
					LOG4CPLUS_ERROR(logger, "stOrderList.SelectFromDB failed, errmsg="<<strErrMsg);
				}
				else
				{
					stOperOrderList = stOrderList.m_oper_order_list;
				}
			}
			else if(!m_user_info.m_oper_id.empty())
			{
				std::vector<OperOrderReverse> stReverseList;
				OperOrderReverseWithOperId stOrderList(m_table_name_oper_order_reverse);
				stOrderList.m_oper_id = m_user_info.m_oper_id;
				if(stOrderList.SelectFromDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
				{
					LOG4CPLUS_ERROR(logger, "stOrderList.SelectFromDB failed, errmsg="<<strErrMsg);
				}
				else
				{
					stReverseList = stOrderList.m_oper_order_reverse_list;
				}

				for(size_t i=0; i!=stReverseList.size(); ++i)
				{
					OperOrder stOperOrder(m_table_name_oper_order);
					stOperOrder.m_id = stReverseList[i].m_oper_order_id;
					if(stOperOrder.SelectFromDB(strErrMsg) == TableBase::TABLE_BASE_RET_OK)
					{
						stOperOrderList.push_back(stOperOrder);
					}
					else
					{
						LOG4CPLUS_ERROR(logger, "stOperOrder.SelectFromDB failed, errmsg="<<strErrMsg);
					}
				}
			}

			return;
		}

		void OpenOrderGet(std::vector<OpenOrder>& stOpenOrderList)
		{
			std::string strErrMsg = "";
			if(m_user_info.m_user_type == USER_TYPE_DEMAND_SIDE)
			{
				OpenOrderWithNeedUin stOrderList(m_table_name_open_order);
				stOrderList.m_need_uin = m_user_info.m_uin;
				if(stOrderList.SelectFromDB(strErrMsg) != TableBase::TABLE_BASE_RET_OK)
				{
					LOG4CPLUS_ERROR(logger, "stOrderList.SelectFromDB failed, errmsg="<<strErrMsg);
				}
				else
				{
					stOpenOrderList = stOrderList.m_open_order_list;
				}
			}

			return;
		}

		
        
		bool InnerProcess()
		{
			std::vector<DevOrder> stDevOrderList;
			std::vector<OperOrder> stOperOrderList;
			std::vector<OpenOrder> stOpenOrderList;

			DevOrderGet(stDevOrderList);
			OperOrderGet(stOperOrderList);
			OpenOrderGet(stOpenOrderList);

			if(!stDevOrderList.empty())
			{
				for(size_t i=0; i!=stDevOrderList.size(); ++i)
				{
					lce::cgi::CAnyValue stAnyValue;
					stDevOrderList[i].ToAnyValue(stAnyValue);
					GetAnyValue()["dev_order_list"].push_back(stAnyValue);
				}
			}

			if(!stOperOrderList.empty())
			{
				for(size_t i=0; i!=stOperOrderList.size(); ++i)
				{
					lce::cgi::CAnyValue stAnyValue;
					stOperOrderList[i].ToAnyValue(stAnyValue);
					GetAnyValue()["oper_order_list"].push_back(stAnyValue);
				}
			}

			if(!stOpenOrderList.empty())
			{
				for(size_t i=0; i!=stOpenOrderList.size(); ++i)
				{
					lce::cgi::CAnyValue stAnyValue;
					stOpenOrderList[i].ToAnyValue(stAnyValue);
					GetAnyValue()["open_order_list"].push_back(stAnyValue);
				}
			}
			
            DoReply(CGI_RET_CODE_OK);
			return true;
		}
};


int main()
{
    CgiUserOrderGet cgi;
    if (!cgi.Run())
    {
        return -1;
    }
    return 0;
}


