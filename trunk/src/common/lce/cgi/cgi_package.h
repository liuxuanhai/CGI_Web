#ifndef _LCE_CGI_PACKAGE_H_
#define _LCE_CGI_PACKAGE_H_

#include <stdexcept>
#include <string>
#include <memory.h>
#include "cgi_utils.h"
#include "cgi_any_value.h"

namespace lce
{
	namespace cgi
	{
		template<typename T>
		class CPackage
		{
		public:
			typedef std::runtime_error Error;
			typedef CPackage this_type;
			typedef T PKG_HEAD;
		public:
			enum SEEK_DIR
			{
				S_BEG,
				S_CUR,
				S_END
			};

			CPackage(void)
			{
				m_iPos = 0;
				m_pkgData.assign(sizeof(PKG_HEAD), 0);
			}

			CPackage(const unsigned char* pszPkgData,const int32_t iLen)
			{
				if (NULL == pszPkgData) 
				{
					throw Error("pszPkgData is null.");
				}
				m_iPos = 0;
				m_pkgData.erase();
				m_pkgData.assign((char*)pszPkgData,iLen);
			}

			~CPackage(void)
			{
			}

			CPackage(const CPackage& rhs)
			{
				this->assign(rhs);
			}

			CPackage& operator= (const CPackage& rhs)
			{
				if (this != &rhs)
				{
					this->assign(rhs);
				}

				return *this;
			}

			void SetPkgData(const unsigned char* pszPkgData,const int32_t iLen)
			{
				this->SetPkgData(reinterpret_cast<const char*>(pszPkgData), iLen);
			}
			void SetPkgData(const char* pszPkgData,const int32_t iLen)
			{
				if (NULL == pszPkgData) 
				{
					throw Error("pszPkgData is null.");
				}
				else if ( static_cast<int32_t>(sizeof(PKG_HEAD)) > iLen )
				{
					throw Error("data len less than pkg head len.");
				}
				m_iPos = 0;
				m_pkgData.erase();
				m_pkgData.assign(pszPkgData,iLen);
			}


			void SetPkgData(const std::string& sData)
			{
				m_iPos = 0;
				m_pkgData = sData;
			}

			//Ð´Êý¾Ý
			void Write(const unsigned char* pszData,const uint16_t wLen)
			{
				m_pkgData.append(reinterpret_cast<const char*>(pszData), wLen);
			}
			void Write(const char* pszData,const uint16_t wLen)
			{
				m_pkgData.append(pszData,wLen);
			}

			size_t Write(const uint32_t dwVal)
			{
				size_t dwPos = m_pkgData.size();
				uint32_t dwTmp = htonl(dwVal);
				m_pkgData.append(reinterpret_cast<const char*>(&dwTmp),sizeof(uint32_t));
				return dwPos;
			}

			size_t Write(const uint16_t wVal)
			{
				size_t dwPos = m_pkgData.size();
				uint16_t wTmp = htons(wVal) ;
				m_pkgData.append(reinterpret_cast<const char*>(&wTmp),sizeof(uint16_t));
				return dwPos;
			}

			size_t Write(const unsigned char ucVal)
			{
				size_t dwPos = m_pkgData.size();
				m_pkgData.append(reinterpret_cast<const char*>(&ucVal),sizeof(unsigned char));
				return dwPos;
			}

			void WritePos(const uint32_t dwVal, const size_t dwPos){
				uint32_t dwTmp = htonl(dwVal) ;
				m_pkgData.replace(dwPos, sizeof(uint32_t), reinterpret_cast<char*>(&dwTmp),sizeof(uint32_t));
			}

			void WritePos(const uint16_t wVal, const size_t dwPos){
				uint16_t wTmp = htons(wVal) ;
				m_pkgData.replace(dwPos, sizeof(uint16_t), reinterpret_cast<char*>(&wTmp),sizeof(uint16_t));
			}

			void WritePos(const unsigned char ucVal, const size_t dwPos){
				m_pkgData.replace(dwPos, sizeof(unsigned char), reinterpret_cast<const char*>(&ucVal),sizeof(unsigned char));
			}

			this_type& operator << (const std::string& sVal)
			{
				m_pkgData += sVal;
				return *this;
			}
			this_type& operator << (const char* pszVal)
			{
				m_pkgData.append(pszVal, strlen(pszVal));
				return *this;
			}
			this_type& operator << (const uint16_t wVal)
			{
				uint16_t wTmp = htons(wVal) ;
				m_pkgData.append(reinterpret_cast<char*>(&wTmp),sizeof(uint16_t));
				return *this;
			}
			this_type& operator << (const int16_t wVal)
			{
				int16_t shTmp = htons(wVal) ;
				m_pkgData.append(reinterpret_cast<char*>(&shTmp),sizeof(int16_t));
				return *this;
			}
			
		#ifdef __x86_64__
		#elif __i386__			
			this_type& operator << (const unsigned long dwVal)
			{
				uint32_t dwTmp = htonl(dwVal);
				m_pkgData.append(reinterpret_cast<char*>(&dwTmp),sizeof(uint32_t));
				return *this;
			}
			this_type& operator << (const long dwVal)
			{
				int32_t dwTmp = htonl(dwVal);
				m_pkgData.append(reinterpret_cast<char*>(&dwTmp),sizeof(int32_t));
				return *this;
			}
		#endif		
			this_type& operator << (const int64_t iVal)
			{
				int64_t iTmp = htonll(iVal);
				m_pkgData.append(reinterpret_cast<char*>(&iTmp),sizeof(int64_t));
				return *this;
			}
			this_type& operator << (const uint64_t uiVal)
			{
				uint64_t uiTmp = htonll(uiVal);
				m_pkgData.append(reinterpret_cast<char*>(&uiTmp),sizeof(uint64_t));
				return *this;
			}	
			
			this_type& operator << (const int32_t iVal)
			{
				int32_t iTmp = htonl(iVal);
				m_pkgData.append(reinterpret_cast<char*>(&iTmp),sizeof(int32_t));
				return *this;
			}
			this_type& operator << (const uint32_t uiVal)
			{
				uint32_t uiTmp = htonl(uiVal);
				m_pkgData.append(reinterpret_cast<char*>(&uiTmp),sizeof(uint32_t));
				return *this;
			}
			template<typename TVal>
				this_type& operator << (const TVal& tVal)
			{
				m_pkgData.append(reinterpret_cast<const char*>(&tVal),sizeof(tVal));
				return *this;
			}

			template<typename TVal>
			this_type& operator >> (TVal& tVal)
			{
				if (sizeof(tVal) <= (m_pkgData.size()-sizeof(PKG_HEAD)-m_iPos))
				{
					memcpy(&tVal,m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos,sizeof(tVal));
				}
				else
				{
					throw Error("data error:no enough buf.");
				}
				m_iPos += static_cast<int32_t>(sizeof(tVal));
				return *this;
			}

			this_type& operator >> (uint16_t& wVal)
			{
				if (sizeof(uint16_t) <= (m_pkgData.size()-sizeof(PKG_HEAD)-m_iPos))
				{
					memcpy(&wVal,m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos,sizeof(wVal));
					wVal = ntohs(wVal);
				}
				else
				{
					throw Error("data error:no enough buf.");
				}
				m_iPos += static_cast<int32_t>(sizeof(uint16_t));
				return *this;
			}
			this_type& operator >> (int16_t& wVal)
			{
				if (sizeof(int16_t) <= (m_pkgData.size()-sizeof(PKG_HEAD)-m_iPos))
				{
					memcpy(&wVal,m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos,sizeof(wVal));
					wVal = ntohs(wVal);
				}
				else
				{
					throw Error("data error:no enough buf.");
				}
				m_iPos += static_cast<int32_t>(sizeof(int16_t));
				return *this;
			}
			this_type& operator >> (uint32_t& uiVal)
			{
				if (sizeof(uint32_t) <= (m_pkgData.size()-sizeof(PKG_HEAD)-m_iPos))
				{
					memcpy(&uiVal,m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos,sizeof(uiVal));
					uiVal = ntohl(uiVal);
				}
				else
				{
					throw Error("data error:no enough buf.");
				}
				m_iPos += static_cast<int32_t>(sizeof(uint32_t));
				return *this;
			}
			this_type& operator >> (int32_t& iVal)
			{
				if (sizeof(int32_t) <= (m_pkgData.size()-sizeof(PKG_HEAD)-m_iPos))
				{
					memcpy(&iVal,m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos,sizeof(iVal));
					iVal = ntohl(iVal);
				}
				else
				{
					throw Error("data error:no enough buf.");
				}
				m_iPos += static_cast<int32_t>(sizeof(int32_t));
				return *this;
			}

				this_type& operator >> (uint64_t& uiVal)
			{
				if (sizeof(uint64_t) <= (m_pkgData.size()-sizeof(PKG_HEAD)-m_iPos))
				{
					memcpy(&uiVal,m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos,sizeof(uiVal));
					uiVal = ntohll(uiVal);
				}
				else
				{
					throw Error("data error:no enough buf.");
				}
				m_iPos += static_cast<int32_t>(sizeof(uint64_t));
				return *this;
			}
			this_type& operator >> (int64_t& iVal)
			{
				if (sizeof(int64_t) <= (m_pkgData.size()-sizeof(PKG_HEAD)-m_iPos))
				{
					memcpy(&iVal,m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos,sizeof(iVal));
					iVal = ntohll(iVal);
				}
				else
				{
					throw Error("data error:no enough buf.");
				}
				m_iPos += static_cast<int32_t>(sizeof(int64_t));
				return *this;
			}	
			this_type& operator >> (CAnyValue &val)
			{
				if (val.GetType() == DType::Integer)
				{
					if (val.GetSubType() == DType::Integer1)
					{
						uint8_t tmp;
						(*this) >> tmp;
						val = tmp;
						return *this;
					}
					else if (val.GetSubType() == DType::Integer2)
					{
						uint16_t tmp;
						(*this) >> tmp;
						val = tmp;
						return *this;
					}
					else if (val.GetSubType() == DType::Integer4)
					{
						uint32_t tmp;
						(*this) >> tmp;
						val = tmp;
						return *this;
					}
					else if (val.GetSubType() == DType::Integer8)
					{
						uint64_t tmp;
						(*this) >> tmp;
						val = tmp;
						return *this;
					}
				}
				else if (val.GetType() == DType::String)
				{
					if (val.GetSubType() == DType::String1)
					{
						uint8_t len;
						(*this) >> len;
						std::string tmp;
						(*this).Read(tmp, len);
						val = tmp;
						return *this;
					}
					else if (val.GetSubType() == DType::String2)
					{
						uint16_t len;
						(*this) >> len;
						std::string tmp;
						(*this).Read(tmp, len);
						val = tmp;
						return *this;
					}
					else if (val.GetSubType() == DType::String4)
					{
						uint32_t len;
						(*this) >> len;
						std::string tmp;
						(*this).Read(tmp, len);
						val = tmp;
						return *this;
					}
				}
				throw Error("can't support CAnyValue type.");
			}	
			
			void Read(char* pszBuf,const int32_t iReadLen)
			{
				if (iReadLen <= static_cast<int32_t>(m_pkgData.size()-sizeof(PKG_HEAD)-m_iPos))
				{
					memcpy(pszBuf,m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos,iReadLen);
				}
				else
				{
					throw Error("data error:no enough buf.");
				}
				m_iPos += iReadLen;
			}

			void Read(std::string& sBuf, const int32_t iReadLen)
			{
				if (iReadLen <= static_cast<int32_t>(m_pkgData.size()-sizeof(PKG_HEAD)-m_iPos))
				{
					sBuf.assign(m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos,iReadLen);
				}
				else
				{
					throw Error("data error:no enough buf.");
				}
				m_iPos += iReadLen;
			}

			void ReadString(std::string& sData){
				const char* pData = m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos;
				int32_t iStrLen = strlen(pData);
				sData = pData;
				m_iPos += iStrLen + 1;
			}

			void ReadString1(std::string& sData){
				if ( 1 <= static_cast<int32_t>(m_pkgData.size()-sizeof(PKG_HEAD)-m_iPos))
				{
					unsigned char ucLen = *(unsigned char*)(m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos);
					++m_iPos;
					Read(sData, ucLen);
				}
				else
				{
					throw Error("data error:no enough buf.");
				}
				
			}

			void ReadString2(std::string& sData){
				if ( 2 <= static_cast<int32_t>(m_pkgData.size()-sizeof(PKG_HEAD)-m_iPos))
				{
					uint16_t wLen = ntohs(*(uint16_t*)(m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos));
					m_iPos += 2;
					Read(sData, wLen);
				}
				else
				{
					throw Error("data error:no enough buf.");
				}

			}

			void ReadString4(std::string& sData){
				unsigned char dwLen = 0;
				if ( 4 <= static_cast<int32_t>(m_pkgData.size()-sizeof(PKG_HEAD)-m_iPos))
				{
					uint32_t dwLen = ntohl(*(uint32_t*)(m_pkgData.data()+sizeof(PKG_HEAD)+m_iPos));
					m_iPos += 4;
					Read(sData, dwLen);
				}
				else
				{
					throw Error("data error:no enough buf.");
				}
			}

			bool IsEnd(const int32_t iReadNum=0)const 
			{
				return  m_pkgData.size() > m_iPos + sizeof(PKG_HEAD)+iReadNum? false : true;
			}

			int32_t GetCurPos() const
			{
				return m_iPos;
			}

			bool Seek(const int32_t iOffPos,const SEEK_DIR pos = S_CUR)
			{
				bool bOk = false;
				switch(pos) {
				case S_BEG:
					if (iOffPos>=0 && static_cast<int32_t>(m_pkgData.size()-sizeof(PKG_HEAD)) > iOffPos) 
					{
						m_iPos = iOffPos;
						bOk = true;
					}
					break;
				case S_CUR:
					if ( static_cast<int32_t>(m_pkgData.size()-sizeof(PKG_HEAD)) > iOffPos && m_iPos+iOffPos>=0)
					{
						m_iPos += iOffPos;
						bOk = true;
					}
					break;
				case S_END:
					if (iOffPos<=0 && (int32_t)m_pkgData.size()>abs(iOffPos))
					{

						m_iPos = (int32_t)m_pkgData.size()-sizeof(PKG_HEAD) + iOffPos;
						bOk = true;
					}
					break;
				}

				return bOk;
			}

			//清除数据
			void Clear()
			{
				m_iPos = 0;
				m_pkgData.assign(sizeof(PKG_HEAD), 0);
			}

			void ClearBody()
			{
				m_iPos = 0;
				m_pkgData.erase(sizeof(PKG_HEAD));
			}

			void SetPkgBody(const unsigned char* pszBody,const int32_t iBodyLen)
			{
				m_pkgData.replace(sizeof(PKG_HEAD),std::string::npos,  reinterpret_cast<const char*>(pszBody),iBodyLen);
			}
			void SetPkgBody(const char* pszBody,const int32_t iBodyLen)
			{
				m_pkgData.replace(sizeof(PKG_HEAD),std::string::npos,  pszBody,iBodyLen);
			}

			void SetPkgHead(T& pkgHead)
			{
				memcpy(const_cast<char*>(m_pkgData.data()),&pkgHead,sizeof(PKG_HEAD));
			}

			const char* data() const {	return m_pkgData.data();	}
			int32_t size() const {	return static_cast<int32_t>(m_pkgData.size()); }

			const char* GetPkg() const {	return m_pkgData.data();	}
			int32_t GetPkgLen() const {	return static_cast<int32_t>(m_pkgData.size());	}
			PKG_HEAD* GetPkgHead() const { return (reinterpret_cast<PKG_HEAD*>(const_cast<char*>(m_pkgData.data())));	}
			PKG_HEAD& head() {	return *(PKG_HEAD*)m_pkgData.data();	}
			const char* GetPkgBody() const {	return m_pkgData.data()+sizeof(PKG_HEAD);	}
			int32_t GetPkgBodyLen() const {	return static_cast<int32_t>(m_pkgData.size()-sizeof(PKG_HEAD));	}
			
		private:
			void assign(const this_type& rhs)
			{
				m_pkgData = rhs.m_pkgData;
				m_iPos = rhs.m_iPos;
			}

			int32_t m_iPos;
			std::string m_pkgData;
		};

	}
}

#endif
