#include "cgi_mysql.h"
#include "mysql.h"
#include "errmsg.h"
#include "mysqld_error.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

namespace lce
{
	namespace cgi
	{
		CMysql::CMysql(void)
			:m_uPort(0)
			,m_pszNULL("")
		{
			m_bConnected=false;
			m_pRes=NULL;
			m_bQuery = false;
			m_sCharsetName = "utf8";
		}

		CMysql::CMysql(const std::string& sHost,const std::string& sDBName,const std::string& sUser,const std::string& sPasswd,uint16_t uPort)
			:m_sHost(sHost)
			,m_sDBName(sDBName)
			,m_sUserName(sUser)
			,m_sPasswd(sPasswd)
			,m_uPort(uPort)
			,m_pszNULL("")
		{
			m_pRes = NULL;
			m_bConnected = false;
			m_bQuery = false;
			m_sCharsetName = "utf8";
		}

		CMysql::~CMysql(void)
		{
			if(m_pRes!=NULL)
			{
				mysql_free_result(m_pRes);
				m_pRes = NULL;
			}

			if(m_bConnected)
			{
				mysql_close(&m_stMySql);
			}
		}

		bool CMysql::SetDatabase(const std::string& sHost, const std::string& sDBName, const std::string& sUser, const std::string& sPasswd, uint16_t uPort)
		{
			if(m_bConnected)
			{
				Disconnect();
			}
			m_sHost=sHost;
			m_sDBName=sDBName;
			m_sUserName=sUser;
			m_sPasswd=sPasswd;
			m_uPort = uPort;

			return true;
		}

		bool CMysql::Init(const std::string& sHost, const std::string& sDBName, const std::string& sUser, const std::string& sPasswd, uint16_t uPort)
		{
			return SetDatabase(sHost, sDBName, sUser, sPasswd, uPort);
		}

		bool CMysql::Connect()
		{
			m_bQuery = false;

			mysql_init(&m_stMySql);
			mysql_options(&m_stMySql, MYSQL_SET_CHARSET_NAME, m_sCharsetName.c_str());
			MYSQL *pMySql=mysql_real_connect(&m_stMySql, m_sHost.c_str(),m_sUserName.c_str(),m_sPasswd.c_str(),m_sDBName.c_str(),m_uPort,NULL,0) ;
			if (pMySql== NULL)
			{
				snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s", mysql_error(&m_stMySql));
				m_uiErrCode=mysql_errno(&m_stMySql);
				return false;
			}
			return true;
		}

		void CMysql::Disconnect()
		{
			if(m_pRes!=NULL)
			{
				mysql_free_result(m_pRes);
				m_pRes=NULL;
			}
			if(m_bConnected)mysql_close(&m_stMySql);
			m_bConnected=false;
			m_bQuery = false;
		}

		bool CMysql::Query(const std::string& sSql)
		{
			for(int i=0; i<2; ++i)
			{
				//connect
				if(!m_bConnected)
				{
					if(!Connect())
					{
						snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s", mysql_error(&m_stMySql));
						m_uiErrCode = mysql_errno(&m_stMySql);
						return false;
					}
					
					m_bConnected=true;
				}

				//clear old query result
				if(m_pRes!=NULL)
				{
					mysql_free_result(m_pRes);
					m_pRes = NULL;
				}

				// do sql
				if(mysql_real_query(&m_stMySql,sSql.c_str(),static_cast<uint32_t>(strlen(sSql.c_str()))) == 0)
				{
					m_pRes=mysql_store_result(&m_stMySql);

					m_dwAffectedRows = static_cast<uint32_t>(mysql_affected_rows(&m_stMySql)); 

					if(m_pRes==NULL)
					{
						m_dwRowCount=0;
						m_uiFieldCount=0;
					}
					else 
					{
						m_dwRowCount = static_cast<uint32_t>(mysql_num_rows(m_pRes));
						m_uiFieldCount = mysql_num_fields(m_pRes);
					}

					m_bQuery = true;
					return true;
				}
				else
				{
					snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s", mysql_error(&m_stMySql));
					m_uiErrCode=mysql_errno(&m_stMySql);
					if(m_uiErrCode == CR_SERVER_GONE_ERROR || m_uiErrCode == CR_SERVER_LOST || m_uiErrCode == CR_UNKNOWN_ERROR)
					{
						//connnect gone away, try to disconnect and  reconnect.
						Disconnect();
						::usleep(10000); //sleep 10ms
						continue;
					}
					else
					{
						//other fail, return false
						return false;
					}
				}
			}

			snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s", "CAOTADAYE, after twice try, still query failed, FUCK!!!!!!");
			return false;
		}


		bool CMysql::Next()
		{
			bool bOk = false;
			if(m_pRes!=NULL && m_dwRowCount>0)
			{
				m_stRow = mysql_fetch_row(m_pRes);
				m_stRowFiledLengths = mysql_fetch_lengths(m_pRes);
				if(m_stRow==NULL)
				{
					snprintf(m_szErrMsg,sizeof(m_szErrMsg),"%s", mysql_error(&m_stMySql));
					m_uiErrCode=mysql_errno(&m_stMySql);
					bOk = false;
				}
				else
				{
					bOk = true;
				}
			}
			
			return bOk;
		}

		const char* CMysql::GetRow(const uint32_t index)
		{
			if ( index < m_uiFieldCount)
			{
				if (NULL == m_stRow[index])
				{
					return m_pszNULL;
				}
				else
				{
					return m_stRow[index];
				}
			}
			else 
			{
				return m_pszNULL;
			}

			return m_pszNULL;
		}

		unsigned long CMysql::GetRowLength(const uint32_t index)
		{
			if ( index < m_uiFieldCount)
			{
				if (NULL == m_stRow[index])
				{
					return 0;
				}
				else
				{
					return m_stRowFiledLengths[index];
				}
			}
			else 
			{
				return 0;
			}

			return 0;
		}

		const char* CMysql::GetRecord(const uint32_t index)
		{
			if ( index < m_uiFieldCount)
			{
				if (NULL == m_stRow[index])
				{
					return m_pszNULL;
				}
				else
				{
					return m_stRow[index];
				}
			}
			else 
			{
				return m_pszNULL;
			}

			return m_pszNULL;
		}

	};

}

