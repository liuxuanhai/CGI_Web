#ifndef _LCE_CGI_MYSQL_H_
#define _LCE_CGI_MYSQL_H_

#include <stdint.h>
#include <string>
#include "mysql.h"
#include "mysqld_error.h"

namespace lce
{
	namespace cgi
	{
		class CMysql
		{
		public:
			CMysql(void);
			CMysql(const std::string& sHost, const std::string& sDBName,const std::string& sUser,const std::string& sPasswd, uint16_t uPort = 3306);
			~CMysql(void);

		public:
			static std::string MysqlEscape(const std::string &str)
			{
				std::string sData;
				sData.resize(str.size()*2+1);
				mysql_escape_string((char*)sData.data(),str.c_str(), static_cast<uint32_t>(str.size())) ;
				return sData.c_str();
			}

			static std::string MysqlEscape(const char* pData, const int32_t iSize)
			{
				std::string sData;
				sData.resize(iSize*2+1);
				mysql_escape_string((char*)sData.data(), pData, static_cast<uint32_t>(iSize)) ;
				return sData.c_str();
			}

			const char* GetRecord(const uint32_t index);
			const char* GetRow(const uint32_t index);
			unsigned long GetRowLength(const uint32_t index);
			bool Next();
			bool Query(const std::string& sSql);

			bool SetDatabase(const std::string& sHost, const std::string& sDBName, const std::string& sUser, const std::string& sPasswd, uint16_t uPort = 3306);
			bool Init(const std::string& sHost, const std::string& sDBName, const std::string& sUser, const std::string& sPasswd, uint16_t uPort = 3306);

			//��ȡ������Ϣ
			const char* GetErrMsg() const{	return m_szErrMsg;	};
			uint32_t GetErrCode() const {	return m_uiErrCode;	};

			//��ȡ�е�����
			uint32_t GetRowCount() const 
			{
				if (m_bQuery) return m_dwRowCount;	
				else return 0;
			};

			uint32_t GetInsertId() { return static_cast<uint32_t>(mysql_insert_id(&m_stMySql));	}
			uint32_t GetAffectedRows()const
			{		
				if (m_bQuery) return m_dwAffectedRows;	
				else return 0;
			};

			//��ȡ�ֶ�����
			uint32_t GetFieldCount() const
			{
				if (m_bQuery) return m_uiFieldCount;	
				else return 0;
			};

			void SetCharsetName(const std::string& sName){
				m_sCharsetName = sName;
			}

		private:
			CMysql(const CMysql& rhs);
			CMysql& operator=(const CMysql& rhs);
		private:
			MYSQL m_stMySql;
			MYSQL_RES *m_pRes;
			MYSQL_ROW m_stRow;
			unsigned long* m_stRowFiledLengths;

			uint32_t m_dwRowCount;	/*�ڽ�������з����е�����*/
			uint32_t m_dwAffectedRows; /*�����ܵ����һ��UPDATE��DELETE��INSERT��ѯӰ��(�仯)������*/
			uint32_t m_uiFieldCount;  /**/

			bool m_bConnected;	/*�Ƿ��������ݿ�*/
			bool m_bQuery;


			std::string m_sHost;
			std::string m_sDBName;
			std::string m_sUserName;
			std::string m_sPasswd;
			uint16_t m_uPort;

			//������Ϣ
			char m_szErrMsg[1024];
			uint32_t m_uiErrCode;
			const char* m_pszNULL;
			std::string m_sCharsetName;
		private:
			inline void Disconnect();
			inline bool Connect();

		};

	}
}

#endif

