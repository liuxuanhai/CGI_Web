#ifndef _LCE_CGI_ANY_VALUE_H_
#define _LCE_CGI_ANY_VALUE_H_

#include <stdexcept>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <memory.h>
#include <assert.h>
#include "cgi_utils.h"
#include <iostream>
#include <deque>
#include <stdint.h>
#include <netinet/in.h>

using namespace std;

namespace lce
{
	namespace cgi
	{
		struct DType
		{
			enum ValueType	//realtype
			{
				Integer1	= 0,		///< tiny uint value (1字节)
				Integer2	= 1,		///< small uint value (2字节)
				Integer4	= 2,		///< unsigned integer value(uint32)(4字节)
				Integer8	= 3,		///< big unsigned interger value(uint64)(8字节)
				Integer		= Integer8,
				String1		= 4,		///< string value	//1个字节表示长度
				String2		= 5,		///< string value	//2个字节表示长度
				String4		= 6,		///< string value	//4个字节表示长度
				String		= String4,
				Vector		= 7,		///< array value (double list)
				Map			= 8,		///< object value (collection of name/value pairs).
				EXT			= 9,			///< 协议定义描述等信息
			}; 

			enum KeyType
			{
				KeyInteger1			= 201,		///< tiny uint value (1字节)
				KeyInteger2			= 202,		///< small uint value (2字节)
				KeyInteger4			= 203,		///< signed integer value(uint32)(4字节)
				KeyInteger8			= 204,		///< big signed interger value(uint64)(8字节)
				KeyInteger			= KeyInteger8,
			};

			enum InnerValueType{
				Null = 255,		///< 'null' value
			};
		};

		struct EncodeType{
			enum ENCODETYPE{
				NORMAL,		//正常编码
				TYPE1,		//优化编码
			};
		};

		class CKey
		{
		public:
			CKey()
				:m_bNum(true)
				,m_nValue(0)
				,m_psValue(NULL)
			{}

			CKey(const std::string& sKey)
				:m_bNum(false)
				,m_nValue(0)
			{
				if ( sKey.size() > 100 )
					m_psValue = new std::string(sKey.substr(0,100));
				else
					m_psValue = new std::string(sKey);
			}

			CKey(const char* pszKey)
				:m_bNum(false)
				,m_nValue(0)
			{
				if ( strlen(pszKey) > 100 )
					m_psValue = new std::string(pszKey, 100);
				else
					m_psValue = new std::string(pszKey);
			}



			CKey(const char nKey)
				:m_bNum(true)
				,m_nValue(nKey)
				,m_psValue(NULL)
			{}

			CKey(const bool nKey)
				:m_bNum(true)
				,m_nValue(nKey?1:0)
				,m_psValue(NULL)
			{}

			CKey(const unsigned char nKey)
				:m_bNum(true)
				,m_nValue(nKey)
				,m_psValue(NULL)
			{}
			CKey(const uint16_t nKey)
				:m_bNum(true)
				,m_nValue(nKey)
				,m_psValue(NULL)
			{}
			CKey(const int16_t nKey)
				:m_bNum(true)
				,m_nValue(nKey)
				,m_psValue(NULL)
			{}
			CKey(const int32_t nKey)
				:m_bNum(true)
				,m_nValue(nKey)
				,m_psValue(NULL)
			{}
			CKey(const uint32_t nKey)
				:m_bNum(true)
				,m_nValue(nKey)
				,m_psValue(NULL)
			{}
			CKey(const int64_t nKey)
				:m_bNum(true)
				,m_nValue(nKey)
				,m_psValue(NULL)
			{}
			CKey(const uint64_t nKey)
				:m_bNum(true)
				,m_nValue(nKey)
				,m_psValue(NULL)
			{}	

			~CKey(){	this->clear();	}

			CKey(const CKey& rhs)
				:m_bNum(rhs.m_bNum)
				,m_nValue(rhs.m_nValue)
				,m_psValue(NULL)
			{
				if ( NULL != rhs.m_psValue )
					m_psValue = new std::string(*rhs.m_psValue);
			}

			CKey& operator=(const CKey& rhs){
				if ( this != &rhs)
				{
					m_bNum = rhs.m_bNum;
					m_nValue = rhs.m_nValue;
					if ( NULL != m_psValue )
					{
						delete m_psValue;
						m_psValue = NULL;
					}
					if ( NULL != rhs.m_psValue )
						m_psValue = new std::string(*rhs.m_psValue);
				}

				return *this;
			}

			void clear(){
				m_bNum = true;
				m_nValue = 0;
				if ( NULL != m_psValue )
				{
					delete m_psValue;
					m_psValue = NULL;
				}
			}

			bool IsNum() const {	return m_bNum;	}
			uint64_t GetNumValue() const {	return m_nValue;	}
			size_t GetStrValueSize() const {	return m_psValue->size();	}
			const char* GetStrValueData() const {	return m_psValue->data();	}

		protected:
			friend bool operator==(const CKey& lhs, const CKey& rhs);
			friend bool operator!=(const CKey& lhs, const CKey& rhs);
			friend bool operator<(const CKey& lhs, const CKey& rhs);
		private:
			bool m_bNum;
			unsigned char ucType;
			uint64_t m_nValue;
			std::string* m_psValue;
		};

		inline bool  operator==(const CKey& lhs, const CKey& rhs){
			return  ( lhs.m_bNum == rhs.m_bNum && (( lhs.m_bNum && lhs.m_nValue == rhs.m_nValue ) || ( !lhs.m_bNum && *lhs.m_psValue == *rhs.m_psValue ))) ? true : false;
		}
		inline bool  operator!=(const CKey& lhs, const CKey& rhs){
			return  ( lhs.m_bNum != rhs.m_bNum || ( lhs.m_bNum && lhs.m_nValue != rhs.m_nValue ) || ( !lhs.m_bNum && *lhs.m_psValue != *rhs.m_psValue )) ? true : false;
		}
		inline bool operator<(const CKey& lhs, const CKey& rhs){
			bool bOk = false;
			if ( lhs.m_bNum == rhs.m_bNum )		
			{
				if ( lhs.m_bNum )
				{
					bOk = lhs.m_nValue < rhs.m_nValue ? true : false;
				} 
				else
				{
					bOk = *lhs.m_psValue < *rhs.m_psValue ? true : false;
				}
			}
			else if ( !lhs.m_bNum < !rhs.m_bNum )
			{
				bOk = true;
			}

			return  bOk;
		}


		class CAnyValue{
		public:
			typedef std::string	BufType;
			typedef CAnyValue this_type;
			typedef std::map<CKey, CAnyValue> MapType;
			typedef std::deque<CAnyValue> VecType;
			typedef std::runtime_error Error;
			typedef std::vector<BufType> VECKEY;

			union ValueHolder{
				uint64_t integer;
				BufType* buf;
				VecType* vec;
				MapType* map;
			};

			typedef void (*ENCODE_FUNC)(std::string&, const ValueHolder&);
			typedef void (*DECODE_FUNC)(size_t& , const char* , const size_t , this_type& );

		public:
			CAnyValue()
				:m_ucType(DType::Null)
				,m_ucSubType(DType::Null)
				,m_bInit(false)
				,m_bHasData(false)
			{
				init();
				m_value.integer = 0;
			}
			CAnyValue(const char cValue)
				:m_ucType(DType::Integer)
				,m_ucSubType(DType::Integer1)
				,m_bInit(false)
				,m_bHasData(cValue==0?false:true)
			{
				init();
				m_value.integer = cValue;
			}

			CAnyValue(const bool bValue)
				:m_ucType(DType::Integer)
				,m_ucSubType(DType::Integer1)
				,m_bInit(false)
				,m_bHasData(bValue)
			{
				init();
				m_value.integer = bValue?1:0;
			}

			CAnyValue(const unsigned char ucValue)
				:m_ucType(DType::Integer)
				,m_ucSubType(DType::Integer1)
				,m_bInit(false)
				,m_bHasData(ucValue==0?false:true)
			{
				init();
				m_value.integer = ucValue;
			}
			CAnyValue(const uint16_t wValue)
				:m_ucType(DType::Integer)
				,m_ucSubType(DType::Integer2)
				,m_bInit(false)
				,m_bHasData(wValue==0?false:true)
			{
				init();
				m_value.integer = wValue;
			}
			CAnyValue(const int16_t shValue)
				:m_ucType(DType::Integer)
				,m_ucSubType(DType::Integer2)
				,m_bInit(false)
				,m_bHasData(shValue==0?false:true)
			{
				init();
				m_value.integer = shValue;
			}
			CAnyValue(const int32_t lValue)
				:m_ucType(DType::Integer)
				,m_ucSubType(DType::Integer4)
				,m_bInit(false)
				,m_bHasData(lValue==0?false:true)
			{
				init();
				m_value.integer = lValue;
			}
			CAnyValue(const uint32_t dwValue)
				:m_ucType(DType::Integer)
				,m_ucSubType(DType::Integer4)
				,m_bInit(false)
				,m_bHasData(dwValue==0?false:true)
			{
				init();
				m_value.integer = dwValue;
			}
			
	#ifdef __x86_64__
	/*
			CAnyValue(const unsigned long dwValue)
				:m_ucType(DType::Integer)
				,m_ucSubType(DType::Integer8)
				,m_bInit(false)
				,m_bHasData(dwValue==0?false:true)
			{
				init();
				m_value.integer = dwValue;
			}
			CAnyValue(const long lValue)
				:m_ucType(DType::Integer)
				,m_ucSubType(DType::Integer8)
				,m_bInit(false)
				,m_bHasData(lValue==0?false:true)
			{
				init();
				m_value.integer = lValue;
			}				
			*/
	#elif __i386__
			CAnyValue(const unsigned long dwValue)
				:m_ucType(DType::Integer)
				,m_ucSubType(DType::Integer4)
				,m_bInit(false)
				,m_bHasData(dwValue==0?false:true)
			{
				init();
				m_value.integer = dwValue;
			}
			CAnyValue(const long lValue)
				:m_ucType(DType::Integer)
				,m_ucSubType(DType::Integer4)
				,m_bInit(false)
				,m_bHasData(lValue==0?false:true)
			{
				init();
				m_value.integer = lValue;
			}		
	#endif

			CAnyValue(const uint64_t ullValue)
				:m_ucType(DType::Integer)
				,m_ucSubType(DType::Integer8)
				,m_bInit(false)
				,m_bHasData(ullValue==0?false:true)
			{
				init();
				m_value.integer = ullValue;
			}

			CAnyValue(const int64_t llValue)
				:m_ucType(DType::Integer)
				,m_ucSubType(DType::Integer8)
				,m_bInit(false)
				,m_bHasData(llValue==0?false:true)
			{
				init();
				m_value.integer = llValue;
			}

			CAnyValue(const char* pszValue)
				:m_ucType(DType::String)
				,m_ucSubType(DType::String4)
				,m_bInit(false)
				,m_bHasData(false)
			{
				init();
				m_value.buf = new BufType(pszValue);
				if ( m_value.buf->size() > 0 )
				{
					m_bHasData = true;
				}
				//if ( m_buf->size() <= 0xFF )
				//	m_ucSubType = DType::String1;
				//else if ( m_buf->size() <= 0xFFFF )
				//	m_ucSubType = DType::String2;
				//else
				//	m_ucSubType = DType::String4;

			}

			CAnyValue(const std::string& sValue)
				:m_ucType(DType::String)
				,m_ucSubType(DType::String4)
				,m_bInit(false)
				,m_bHasData(false)
			{
				init();
				m_value.buf  = new BufType(sValue);
				if ( m_value.buf->size() > 0 )
				{
					m_bHasData = true;
				}
			}

			CAnyValue(const CAnyValue& rhs)
				:m_bInit(false)
			{
				init();
				assign(rhs);
				//m_bHasData = rhs.m_bHasData;
				//m_ucSubType = rhs.m_ucSubType;
				//m_ucType = rhs.m_ucType;
				//m_value = rhs.m_value;
			}
			CAnyValue& operator=(const CAnyValue& rhs){
				if ( this != &rhs )
				{
					assign(rhs);
				}

				return *this;
			}
			
			CAnyValue &InitAsInt1()
			{
				if (DType::Null != m_ucType)
				{
					throw Error("CAnyValue is already inited.");
				}
				m_ucType = DType::Integer;
				m_ucSubType = DType::Integer1;
				return *this;
			}
			CAnyValue &InitAsInt2()
			{
				if (DType::Null != m_ucType)
				{
					throw Error("CAnyValue is already inited.");
				}
				m_ucType = DType::Integer;
				m_ucSubType = DType::Integer2;
				return *this;
			}
			CAnyValue &InitAsInt4()
			{
				if (DType::Null != m_ucType)
				{
					throw Error("CAnyValue is already inited.");
				}
				m_ucType = DType::Integer;
				m_ucSubType = DType::Integer4;
				return *this;
			}
			CAnyValue &InitAsInt8()
			{
				if (DType::Null != m_ucType)
				{
					throw Error("CAnyValue is already inited.");
				}
				m_ucType = DType::Integer;
				m_ucSubType = DType::Integer8;
				return *this;
			}
			CAnyValue &InitAsStr1()
			{
				if (DType::Null != m_ucType)
				{
					throw Error("CAnyValue is already inited.");
				}
				m_ucType = DType::String;
				m_ucSubType = DType::String1;
				m_value.buf = new BufType("");
				m_bHasData = true;
				return *this;
			}
			CAnyValue &InitAsStr2()
			{
				if (DType::Null != m_ucType)
				{
					throw Error("CAnyValue is already inited.");
				}
				m_ucType = DType::String;
				m_ucSubType = DType::String2;
				m_value.buf = new BufType("");
				m_bHasData = true;
				return *this;
			}
			CAnyValue &InitAsStr4()
			{
				if (DType::Null != m_ucType)
				{
					throw Error("CAnyValue is already inited.");
				}
				m_ucType = DType::String;
				m_ucSubType = DType::String4;
				m_value.buf = new BufType("");
				m_bHasData = true;
				return *this;
			}

		public:
		
			DType::ValueType GetType() const {	return (DType::ValueType)m_ucType;	}
			DType::ValueType GetSubType() const { return (DType::ValueType)m_ucSubType; }

			int asInt() const {	
				if ( DType::Integer == m_ucType )	return static_cast<int>(m_value.integer);
				return 0;	
			}

			std::string asString() const {	
				if ( DType::String == m_ucType && NULL != m_value.buf  )		return std::string(m_value.buf->data(), m_value.buf->size());
				return m_strNull;
			}

			operator uint64_t() const {	
				if ( DType::Integer == m_ucType )	return m_value.integer;
				return 0;	
			}
			operator int64_t() const {	
				if ( DType::Integer == m_ucType )	return static_cast<int64_t>(m_value.integer);
				return 0;	
			}
			operator unsigned char() const {	
				if ( DType::Integer == m_ucType )	return static_cast<unsigned char>(m_value.integer);
				return 0;	
			}
			//operator char() const {	
			//	if ( DType::Integer == m_ucType )	return static_cast<char>(m_value.integer);
			//	return 0;	
			//}
			operator uint16_t() const {	
				if ( DType::Integer == m_ucType )	return static_cast<uint16_t>(m_value.integer);
				return 0;	
			}
			operator int16_t() const {	
				if ( DType::Integer == m_ucType )	return static_cast<int16_t>(m_value.integer);
				return 0;	
			}
			operator uint32_t() const {		
				if ( DType::Integer == m_ucType )	return static_cast<uint32_t>(m_value.integer);
				return 0;	
			}
			operator int32_t() const {	
				if ( DType::Integer == m_ucType )	return static_cast<int32_t>(m_value.integer);
				return 0;	
			}
	#ifdef __x86_64__
	#elif __i386__
			operator long() const {	
				if ( DType::Integer == m_ucType )	return static_cast<long>(m_value.integer);
				return 0;	
			}
			operator unsigned long() const {	
				if ( DType::Integer == m_ucType )	return static_cast<unsigned long>(m_value.integer);
				return 0;	
			}
	#endif	
			operator bool() const {	
				if ( DType::Integer == m_ucType )	return m_value.integer==0 ? false : true;
				return false;	
			}
			// operator const std::string() const {	
				// if ( DType::String == m_ucType && NULL != m_value.buf  )		return std::string(m_value.buf->data(), m_value.buf->size());
				// return m_strNull;
			// }

			operator std::string() const {	
				if ( DType::String == m_ucType && NULL != m_value.buf  )		return std::string(m_value.buf->data(), m_value.buf->size());
				return m_strNull;
			}
			
			const char* data() const {	
				if ( DType::String == m_ucType && NULL != m_value.buf  )		return m_value.buf->data();
				return m_strNull.data();

			}
			const size_t size()	const {	
				if ( m_ucType == DType::String ){
					if ( NULL != m_value.buf ){
						return m_value.buf->size();	
					}
					else{
						assert(false);
					}
				}
				else if ( m_ucType == DType::Vector ){
					if ( NULL != m_value.vec )
						return m_value.vec->size();
					else
						assert(false);
				}
				else if ( m_ucType == DType::Map ){
					if ( NULL != m_value.map )
						return m_value.map->size();
					else
						assert(false);
				}
				return 0;	
			}

		public:
			const CAnyValue& row(const int iIndex) const {
				if ( (DType::Vector==m_ucType) && NULL != m_value.vec )
				{
					if ( iIndex < (int)m_value.vec->size() )
						return (*m_value.vec)[iIndex];
				}
				return m_null; 
			}

			CAnyValue& row(const int iIndex) {
				if ( (DType::Vector==m_ucType) && NULL != m_value.vec )
				{
					if ( iIndex < (int)m_value.vec->size() )
						return (*m_value.vec)[iIndex];
					else throw Error("row: index invalid");
				}
				else throw Error("row: type invalid");
				//		return m_null; 
			}

			
			const CAnyValue& operator[](const CKey& oKey) const
			{
				if ( (DType::Map == m_ucType) && NULL != m_value.map )
				{
					MapType::iterator it = m_value.map->find(oKey);
					if ( it != m_value.map->end() )
						return it->second;
				}
				return m_null;
			}

			const CAnyValue& operator[](const int32_t iKey) const 
			{
				return this->find(iKey);
			} 

			const CAnyValue& operator[](const char* pKey) const
			{
				return this->find(pKey);
			} 

			const CAnyValue& operator[](const std::string& sKey) const
			{
				return this->find(sKey);
			} 

			const CAnyValue& operator[](const uint64_t uKey) const
			{
				return this->find(uKey);
			} 

			CAnyValue& operator[](const uint64_t uKey)
			{
				return this->find(uKey);
			} 

			CAnyValue& operator[](const int32_t iKey)
			{
				return this->find(iKey);
			} 

			CAnyValue& operator[](const char* pKey)
			{
				return this->find(pKey);
			} 

			CAnyValue& operator[](const std::string& sKey)
			{
				return this->find(sKey);
			} 

			CAnyValue& operator[](const CKey& oKey)
			{
				return this->find(oKey);
			}

			const CAnyValue& find(const CKey& oKey) const
			{
				if ( (DType::Map == m_ucType) && NULL != m_value.map )
				{
					MapType::iterator it = m_value.map->find(oKey);
					if ( it != m_value.map->end() )
						return it->second;
				}
				return m_null;
			}

			CAnyValue& find(const CKey& oKey)
			{
				this->InitAsMap();
				if ( m_value.map == NULL )
				{
					throw Error("anyValue type error: no map type.");
				}	
				MapType::iterator it = m_value.map->find(oKey);
				if ( it == m_value.map->end() )
				{
					m_bHasData = true;
					it = m_value.map->insert( MapType::value_type(oKey, CAnyValue()) ).first;
				}
				return it->second;
			}

			bool hasKey(const CKey& oKey) const {
				if ( (DType::Map == m_ucType) && NULL != m_value.map )
				{
					MapType::iterator it = m_value.map->find(oKey);
					if ( it != m_value.map->end() )
						return true;
				}
				return false;
			}

			//map
			void insert(const CKey& oKey, const CAnyValue& oValue)
			{
				this->InitAsMap();
				if ( (DType::Map == m_ucType) && NULL != m_value.map )
				{
					m_bHasData = true;
					(*m_value.map)[oKey] = oValue;
				}
			}

			//vector
			void push_back(const CAnyValue& oValue){
				this->InitAsVector();
				if ( (DType::Vector == m_ucType) && NULL != m_value.vec )
				{
					m_bHasData = true;
					m_value.vec->push_back(oValue);
				}
			}
			void pop_back(){
				this->InitAsVector();
				if ( (DType::Vector == m_ucType) && NULL != m_value.vec )
				{
					m_bHasData = true;
					m_value.vec->pop_back();
				}
			}
			void push_front(const CAnyValue& oValue){
				this->InitAsVector();
				if ( (DType::Vector == m_ucType) && NULL != m_value.vec )
				{
					m_bHasData = true;
					m_value.vec->push_front(oValue);
				}
			}
			void pop_front(){
				this->InitAsVector();
				if ( (DType::Vector == m_ucType) && NULL != m_value.vec )
				{
					m_bHasData = true;
					m_value.vec->pop_front();
				}
			}

			void erase(const CKey& oKey)
			{
				if ( (DType::Map == m_ucType) && NULL != m_value.map )
				{
					m_value.map->erase(oKey);
				}
			}

			void reset(){
				clear();
			}

			void clear(){

	//			switch(m_ucType)
	//			{
	//			case DType::String:
	//				if ( NULL != m_value.buf ){
	////					m_value.buf->clear();
	//					delete m_value.buf;
	//					m_value.buf = NULL;
	//				}
	//				break;
	//			case DType::Vector:
	//				if ( NULL != m_value.vec ){
	////					m_value.vec->clear();
	//					delete m_value.vec;
	//					m_value.vec = NULL;
	//				}
	//				break;
	//			case DType::Map:
	//				if ( NULL != m_value.map ){
	////					m_value.map->clear();
	//					delete m_value.map;
	//					m_value.map = NULL;
	//				}
	//				break;
	//			}
	//			m_bHasData = false;
	//			m_value.integer = 0;
	//			m_ucType = DType::Null;
	//			m_ucSubType = DType::Null;

				if ( m_bInit ){
					switch(m_ucType)
					{
					case DType::String:
						if ( NULL != m_value.buf ){
							delete m_value.buf;
							m_value.buf = NULL;
						}
						break;
					case DType::Vector:
						if ( NULL != m_value.vec ){
							delete m_value.vec;
							m_value.vec = NULL;
						}
						break;
					case DType::Map:
						if ( NULL != m_value.map ){
							delete m_value.map;
							m_value.map = NULL;
						}
						break;
					}
				}else{
					assert(false);
				}

				memset(&m_value, 0, sizeof(m_value));
				m_ucType = DType::Null;
				m_ucSubType = DType::Null;
				m_bHasData = false;
			}


			~CAnyValue(){
				this->clear();
			}

			static void decode_integer1(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj){
				thisobj.m_ucType = DType::Integer;
				check(dwDecodePos+1, dwDataSize);
				thisobj.m_value.integer = (unsigned char)*(unsigned char*)(pData+dwDecodePos);
				++dwDecodePos;

			}
			static void decode_integer2(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj){
				thisobj.m_ucType = DType::Integer;
				check(dwDecodePos+2, dwDataSize);
				thisobj.m_value.integer = ntohs(*(uint16_t*)(pData+dwDecodePos));
				dwDecodePos += 2;
			}
			static void decode_integer4(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj){
				thisobj.m_ucType = DType::Integer;
				check(dwDecodePos+4, dwDataSize);
				thisobj.m_value.integer = ntohl(*(uint32_t*)(pData+dwDecodePos));
				dwDecodePos += 4;
			}
			static void decode_integer8(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj){
				thisobj.m_ucType = DType::Integer;
				check(dwDecodePos+8, dwDataSize);
				//		thisobj.m_value.integer = (*(long long*)(pData+dwDecodePos));

				thisobj.m_value.integer = ntohll(*(uint64_t*)(pData+dwDecodePos));
				dwDecodePos += 8;
			}
			static void decode_string1(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj){
				thisobj.InitAsBuf();
				check(dwDecodePos+1, dwDataSize);
				unsigned char ucStrLen = *(unsigned char*)(pData+dwDecodePos);
				++dwDecodePos;
				check(dwDecodePos+ucStrLen, dwDataSize);
				thisobj.m_value.buf->assign(pData+dwDecodePos, ucStrLen);
				dwDecodePos += ucStrLen;
			}
			static void decode_string2(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj){
				thisobj.InitAsBuf();
				check(dwDecodePos+2, dwDataSize);
				uint16_t wStrLen = ntohs(*(uint16_t*)(pData+dwDecodePos));
				dwDecodePos += 2;
				check(dwDecodePos+wStrLen, dwDataSize);
				thisobj.m_value.buf->assign(pData+dwDecodePos, wStrLen);
				dwDecodePos += wStrLen;
			}
			static void decode_string4(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj){
				thisobj.InitAsBuf();
				check(dwDecodePos+4, dwDataSize);
				uint32_t dwStrLen = ntohl(*(uint32_t*)(pData+dwDecodePos));
				dwDecodePos += 4;
				check(dwDecodePos+dwStrLen, dwDataSize);
				thisobj.m_value.buf->assign(pData+dwDecodePos, dwStrLen);
				dwDecodePos += dwStrLen;
			}
			static void decode_vector(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj){
				thisobj.InitAsVector();
				check(dwDecodePos+4, dwDataSize);
				uint32_t dwSize = ntohl(*(uint32_t*)(pData+dwDecodePos));
				dwDecodePos += 4;
				while ( dwSize > 0 )
				{
	//				cout << "dwSize=" << dwSize << endl;
					--dwSize;
					CAnyValue value;
					value.decode(dwDecodePos, pData, dwDataSize);
					thisobj.m_value.vec->push_back(value);

				}
			}
			static void decode_map(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj){
				thisobj.InitAsMap();
				check(dwDecodePos+4, dwDataSize);
				uint32_t dwSize = ntohl(*(uint32_t*)(pData+dwDecodePos));
				dwDecodePos += 4;
				while ( dwSize > 0 )
				{
					--dwSize;
					check(dwDecodePos+1, dwDataSize);
					unsigned char ucNameLen =  *(unsigned char*)(pData+dwDecodePos);
					++dwDecodePos;
					CKey oKey;
					if ( ucNameLen <= 100 )
					{
						check(dwDecodePos+ucNameLen, dwDataSize);
						std::string sName(pData+dwDecodePos, ucNameLen);
						dwDecodePos += ucNameLen;
						oKey = sName;
					}
					else if ( ucNameLen == DType::KeyInteger1 )
					{
						check(dwDecodePos+1, dwDataSize);
						oKey = (unsigned char)*(unsigned char*)(pData+dwDecodePos);
						++dwDecodePos;
					}
					else if ( ucNameLen == DType::KeyInteger2 )
					{
						check(dwDecodePos+2, dwDataSize);
						oKey = ntohs(*(uint16_t*)(pData+dwDecodePos));
						dwDecodePos += 2;
					}
					else if ( ucNameLen == DType::KeyInteger4 )
					{
						check(dwDecodePos+4, dwDataSize);
						oKey = ntohl(*(uint32_t*)(pData+dwDecodePos));
						dwDecodePos += 4;
					}
					else if ( ucNameLen == DType::KeyInteger8 )
					{
						check(dwDecodePos+8, dwDataSize);
						oKey = ntohll(*(uint64_t*)(pData+dwDecodePos));
						dwDecodePos += 8;
					}

					if ( dwDataSize > dwDecodePos )
					{
						CAnyValue value;
						value.decode(dwDecodePos, pData, dwDataSize);
						thisobj.m_value.map->insert( MapType::value_type(oKey, value) );
					}
				}
			}
			static void decode_ext(size_t& dwDecodePos, const char* pData, const size_t dwDataSize, this_type& thisobj){
				//check(dwDecodePos+1, dwDataSize);
				//unsigned char ucType = *(unsigned char*)(pData+dwDecodePos);
				//++dwDecodePos;

	//			assert(false);
			}

			void decode(size_t& dwDecodePos, const char* pData, const size_t dwDataSize){
				static DECODE_FUNC arDecodeFunc[10] ={&decode_integer1, &decode_integer2, &decode_integer4, &decode_integer8,&decode_string1,
					&decode_string2, &decode_string4, &decode_vector, &decode_map,  &decode_ext};
				check(dwDecodePos+1, dwDataSize);
				m_ucSubType = *(pData+dwDecodePos);
				dwDecodePos++;

				if ( m_ucSubType< 10 ){
					arDecodeFunc[m_ucSubType](dwDecodePos, pData, dwDataSize, *this);
				}
			}

			static void encode_integer(std::string& sBuf, const ValueHolder& value){
				if ( value.integer <= 0xFF ){
					sBuf.push_back((char)DType::Integer1);
					sBuf.push_back((char)value.integer);
				}
				else if ( value.integer <= 0xFFFF ){
					sBuf.push_back((char)DType::Integer2);
					uint16_t wTmp = htons(static_cast<uint16_t>(value.integer));
					sBuf.append(reinterpret_cast<char*>(&wTmp),sizeof(wTmp));
				}
				else if ( value.integer <= 0xFFFFFFFF ){
					sBuf.push_back((char)DType::Integer4);
					uint32_t dwTmp = htonl(static_cast<uint32_t>(value.integer));
					sBuf.append(reinterpret_cast<char*>(&dwTmp),sizeof(dwTmp));
				}
				else{
					sBuf.push_back((char)DType::Integer8);
					uint64_t ui64Tmp = htonll(value.integer);
					sBuf.append(reinterpret_cast<char*>(&ui64Tmp),sizeof(ui64Tmp));
				}
			}

			static void encode_string(std::string& sBuf, const ValueHolder& value){
				if ( value.buf->size() <= 0xFF ){
					sBuf.push_back((char)DType::String1);
					sBuf += static_cast<char>(value.buf->size());
				}
				else if ( value.buf->size() <= 0xFFFF ){
					sBuf.push_back((char)DType::String2);
					uint16_t wSize = htons(static_cast<uint16_t>(value.buf->size()));
					sBuf.append(reinterpret_cast<char*>(&wSize), sizeof(wSize));
				}
				else{
					sBuf.push_back((char)DType::String4);
					uint32_t dwSize = htonl(static_cast<uint32_t>(value.buf->size()));
					sBuf.append(reinterpret_cast<char*>(&dwSize), sizeof(dwSize));
				}
				sBuf.append(value.buf->data(), value.buf->size());
			}
			static void encode_vector(std::string& sBuf, const ValueHolder& value){
				sBuf.push_back((char)DType::Vector);
				uint32_t dwSize = htonl(static_cast<uint32_t>(value.vec->size()));
				sBuf.append(reinterpret_cast<char*>(&dwSize), sizeof(dwSize));
				for(VecType::iterator it=value.vec->begin(); it!=value.vec->end(); ++it)
				{
					it->encode(sBuf);
				}
			}
			static void encode_map(std::string& sBuf, const ValueHolder& value){
				sBuf.push_back((char)DType::Map);
				size_t dwSizePos = sBuf.size();
				uint32_t dwSize = 0;//htonl(static_cast<uint32_t>(value.map->size()));
				sBuf.append(reinterpret_cast<char*>(&dwSize), sizeof(dwSize));
				for(MapType::iterator it=value.map->begin(); it!=value.map->end(); ++it)
				{
					if ( it->second.m_ucType != DType::Null )
					{
						++dwSize;
						encode_key(sBuf, it->first);
						it->second.encode(sBuf);
					}
				}
				dwSize = htonl(dwSize);
				memcpy((char*)sBuf.data()+dwSizePos, reinterpret_cast<char*>(&dwSize), sizeof(dwSize));
			}
			static void encode_key(std::string& sBuf, const CKey& oKey){

				if ( oKey.IsNum() )
				{
					if ( oKey.GetNumValue() <= 0xFF ){
						sBuf.push_back((char)DType::KeyInteger1);
						sBuf.push_back((char)oKey.GetNumValue());
					}
					else if ( oKey.GetNumValue() <= 0xFFFF ){
						sBuf.push_back((char)DType::KeyInteger2);
						uint16_t wTmp = htons(static_cast<uint16_t>(oKey.GetNumValue()));
						sBuf.append(reinterpret_cast<char*>(&wTmp),sizeof(wTmp));
					}
					else if ( oKey.GetNumValue() <= 0xFFFFFFFF ){
						sBuf.push_back((char)DType::KeyInteger4);
						uint32_t dwTmp = htonl(static_cast<uint32_t>(oKey.GetNumValue()));
						sBuf.append(reinterpret_cast<char*>(&dwTmp),sizeof(dwTmp));
					}
					else{
						sBuf.push_back((char)DType::KeyInteger8);
						uint64_t ui64Tmp = htonll(oKey.GetNumValue());
						sBuf.append(reinterpret_cast<char*>(&ui64Tmp),sizeof(ui64Tmp));
					}
				} 
				else
				{
					sBuf.push_back((char)oKey.GetStrValueSize());
					sBuf.append(oKey.GetStrValueData(), oKey.GetStrValueSize());
				}
			}


			static void encode_none(std::string& sBuf, const ValueHolder& value){
				//sBuf.push_back((char)DType::EXT);
				//char ucType = 0;
				//sBuf += ucType;
	//			assert(false);
			}

			void encode(std::string& sBuf){
				//static EnCODE_FUNC arDecodeFunc[12] ={&encode_none, &encode_none, &encode_none, &encode_integer,
				//									  &encode_none, &encode_none, &encode_string, &encode_vector, 
				//									  &encode_map,  &encode_none, &encode_none, &encode_none};

				//if ( m_ucType<=8 ){
				//	arEncodeFunc[m_ucType](sBuf, m_value);
				//}

				switch(m_ucType){
			case DType::Integer:
				encode_integer(sBuf, m_value);
				break;
			case DType::String:
				encode_string(sBuf, m_value);
				break;
			case DType::Vector:
				encode_vector(sBuf, m_value);
				break;
			case DType::Map:
				encode_map(sBuf, m_value);
				break;
			default:
				encode_none(sBuf, m_value);
				break;
				}
			}

			void encodeXML(std::string& sBuf,const bool bUtf8=true,const bool bEncodeJS=true){
				switch(m_ucType)
				{
				case DType::Integer:
					{
						sBuf += ToStr(m_value.integer);
					}
					break;
				case DType::String:
					{
						sBuf += "<![CDATA[";
						if ( bEncodeJS )
							sBuf += TextEncodeJS(string(m_value.buf->data(), m_value.buf->size()), bUtf8);
						else
							sBuf.append(m_value.buf->data(), m_value.buf->size());
						sBuf += "]]>";
					}
					break;
				case DType::Vector:
					{
						for(VecType::iterator it=m_value.vec->begin(); it!=m_value.vec->end(); ++it)
						{
							sBuf += "<l>";
							it->encodeXML(sBuf, bUtf8, bEncodeJS);
							sBuf += "</l>";
						}
					}
					break;
				case DType::Map:
					{
						for(MapType::iterator it=m_value.map->begin(); it!=m_value.map->end(); ++it)
						{
							sBuf += "<";
							if ( bEncodeJS )
								sBuf += TextEncodeJS(encode_keyByStrXML(it->first), bUtf8);
							else
								sBuf += encode_keyByStrXML(it->first);
							sBuf += ">";
							it->second.encodeXML(sBuf, bUtf8, bEncodeJS);
							sBuf += "</";
							if ( bEncodeJS )
								sBuf += TextEncodeJS(encode_keyByStrXML(it->first), bUtf8);
							else
								sBuf += encode_keyByStrXML(it->first);
							sBuf += ">";
						}
					}
					break;
				default:
					break;
				}
			}

			static std::string encode_keyByStrXML(const CKey& oKey){
				if ( oKey.IsNum() )
				{
					return "_" + ToStr(oKey.GetNumValue());
				} 
				else
				{
					return oKey.GetStrValueData();
				}
				return "";
			}

			static std::string encode_keyByStr(const CKey& oKey){
				if ( oKey.IsNum() )
				{
					return ToStr(oKey.GetNumValue());
				} 
				else
				{
					return oKey.GetStrValueData();
				}
				return "";
			}




			void encodeJSON(std::string& sBuf, const bool bUtf8=true,const bool bEncodeJS=true){
				switch(m_ucType)
				{
				case DType::Integer:
					{
						sBuf += ToStr(m_value.integer);
					}
					break;
				case DType::String:
					{
						sBuf += "\"";
						if ( bEncodeJS )
							sBuf += TextEncodeJSON(TextEncodeJS(string(m_value.buf->data(), m_value.buf->size()), bUtf8), bUtf8);
						else
							sBuf += TextEncodeJSON(string(m_value.buf->data(), m_value.buf->size()), bUtf8);
		
						sBuf += "\"";
					}
					break;
				case DType::Vector:
					{
						bool bFirst = true;
						sBuf += "[";
						for(VecType::iterator it=m_value.vec->begin(); it!=m_value.vec->end(); ++it)
						{
							if ( !bFirst ){
								sBuf += ",";
							}
							else
							{
								bFirst = false;
							}
							it->encodeJSON(sBuf, bUtf8, bEncodeJS);
						}
						sBuf += "]";
					}
					break;
				case DType::Map:
					{
						sBuf += "{";
						bool bFirst = true;
						for(MapType::iterator it=m_value.map->begin(); it!=m_value.map->end(); ++it)
						{
							if ( bFirst ){
								sBuf += "\""; bFirst = false;
							}else{
								sBuf += ",\"";
							}
							//sBuf.append(it->first.data(), it->first.size());
							if ( bEncodeJS )
								sBuf += TextEncodeJSON(TextEncodeJS(encode_keyByStr(it->first), bUtf8), bUtf8);
							else
								sBuf += TextEncodeJSON(encode_keyByStr(it->first), bUtf8);

							sBuf += "\":";
							it->second.encodeJSON(sBuf, bUtf8, bEncodeJS);
						}
						sBuf += "}";
					}
					break;
				default:
					{
						sBuf += "\"\"";
					}
					break;
				}
			}


			static std::vector<DECODE_FUNC> m_vecDecodeFuncs;
			static const CAnyValue m_null;
			static const std::string m_strNull;
			static void check(const size_t dwPos, const size_t sDataSize){
				if ( dwPos >  sDataSize ){
					throw Error("decode data error.");
				}
			}
		private:
			bool isHasData() const {	return m_bHasData;	}
			void init(){
				if ( !m_bInit )
				{
					memset(&m_value, 0, sizeof(m_value));
					m_bInit = true;
				}
			}
			void InitAsMap(){
				init();
				assert(DType::Null==m_ucType || DType::Map==m_ucType);
				if ( DType::Null==m_ucType || DType::Map==m_ucType )
				{
					if ( NULL == m_value.map )
					{
						m_ucType = DType::Map;
						m_ucSubType = DType::Map;
						m_value.map = new MapType;
					}
				}
				else
				{
					throw Error("InitAsMap error:type error.");
				}
			}
			void InitAsVector(){
				init();
				assert(DType::Null==m_ucType || DType::Vector==m_ucType);
				if ( DType::Null==m_ucType || DType::Vector==m_ucType )
				{
					if ( NULL == m_value.vec )
					{
						m_ucType = DType::Vector;
						m_ucSubType = DType::Vector;
						m_value.vec = new VecType;
					}
				}
				else
				{
					throw Error("InitAsVector error:type error.");
				}
			}
			void InitAsBuf(){
				init();
				assert(DType::Null==m_ucType || DType::String==m_ucType);
				if ( DType::Null==m_ucType || DType::String==m_ucType )
				{
					if ( NULL == m_value.buf )
					{
						m_ucType = DType::String;
						m_ucSubType = DType::String;
						m_value.buf = new BufType;
					}
				}
				else
				{
					throw Error("InitAsBuf error:type error.");
				}
			}

			void assign(const this_type& rhs){

				if ( m_ucType == DType::String ) delete m_value.buf;
				if ( m_ucType == DType::Vector ) delete m_value.vec;
				if ( m_ucType == DType::Map ) delete m_value.map;

				if ( rhs.m_ucType == DType::Integer ){
					m_value.integer = rhs.m_value.integer;
				}
				else if ( rhs.m_ucType == DType::Map ){
					assert(rhs.m_value.map != NULL);
					m_value.map = new MapType(*(rhs.m_value.map));
				}
				else if ( DType::Vector ==  rhs.m_ucType )
				{
					assert(rhs.m_value.vec != NULL);
					m_value.vec = new VecType(*rhs.m_value.vec);
				}
				else if ( DType::String ==  rhs.m_ucType )
				{
					assert(rhs.m_value.buf != NULL);
					m_value.buf = new BufType((*rhs.m_value.buf));
				}
				m_ucType = rhs.m_ucType;
				m_ucSubType = rhs.m_ucSubType;
				m_bHasData = rhs.m_bHasData;
				m_bInit = rhs.m_bInit;
			}

		private:
			unsigned char m_ucType;
			unsigned char m_ucSubType;
			ValueHolder m_value;
			bool m_bInit;
			bool m_bHasData;
		};


		template<typename T>
		class CAnyValuePackage{
		public:
			typedef std::runtime_error Error;
			typedef CAnyValuePackage this_type;
			typedef T PKG_HEAD;
		public:
			CAnyValuePackage(){
				m_sDecodeData.assign(sizeof(PKG_HEAD), 0);
				m_sEncodeData.assign(sizeof(PKG_HEAD), 0);
			}

			CAnyValuePackage(const unsigned char* pszPkgData,const int32_t iLen){
				this->CAnyValuePackage((char*)pszPkgData, iLen);
			}


			CAnyValuePackage(const char* pszPkgData,const int32_t iLen)
			{
				if (NULL == pszPkgData) 
				{
					throw Error("pszPkgData is null.");
				}
				if ( iLen < sizeof(PKG_HEAD) )
				{
					throw Error("data no enough len .");
				}
				m_sDecodeData.erase();
				m_sDecodeData.assign((char*)pszPkgData,iLen);
				m_sEncodeData.assign((char*)pszPkgData, sizeof(PKG_HEAD));
			}

			PKG_HEAD& head(){	return *(PKG_HEAD*)m_sEncodeData.data();	}
			void sethead(const PKG_HEAD& stHead)	{	
				memcpy((char*)m_sEncodeData.data(), &stHead, sizeof(stHead)); 
				memcpy((char*)m_sDecodeData.data(), &stHead, sizeof(stHead)); 
			}

			~CAnyValuePackage(){}

			void encodeJSON(const bool bUtf8=true,const bool bEncodeJS=true){
				m_sEncodeData.erase(sizeof(PKG_HEAD));
				m_sEncodeData.assign(m_sDecodeData.data(), sizeof(PKG_HEAD));
				m_oAnyValues.encodeJSON(m_sEncodeData, bUtf8, bEncodeJS);
			}
			void encodeXML(const bool bUtf8=true, const bool bEncodeJS=true){
				m_sEncodeData.erase(sizeof(PKG_HEAD));
				m_sEncodeData.assign(m_sDecodeData.data(), sizeof(PKG_HEAD));
				if (bUtf8)
					m_sEncodeData += "<?xml version=\"1.0\" encoding=\"UTF-8\"?><anyvalue>";
				else
					m_sEncodeData += "<?xml version=\"1.0\" encoding=\"GB2312\"?><anyvalue>";
				m_oAnyValues.encodeXML(m_sEncodeData, bUtf8, bEncodeJS);
				m_sEncodeData += "</anyvalue>";
			}


			void encode(const unsigned char ucEncodeType=EncodeType::NORMAL){
				//清除包体数据
				m_sEncodeData.erase(sizeof(PKG_HEAD));
				m_sEncodeData.append((char*)&ucEncodeType, sizeof(ucEncodeType));
				//正常编解码
				m_oAnyValues.encode(m_sEncodeData);
			}

			void decode(const std::string& sData)
			{
				this->decode(sData.data(), sData.size());
			}
			void decode(const unsigned char* pData, const size_t dwDataSize){
				this->decode((char*)pData, dwDataSize);
			}
			void decode(const char* pData, const size_t dwDataSize){

				if ( dwDataSize < sizeof(PKG_HEAD) ){
					assert(false);
					throw Error("decode error:dwDataSize < sizeof(PKG_HEAD)");
				}
				m_dwPos = 0;
				m_oAnyValues.clear();
				m_sDecodeData.assign(pData, dwDataSize);
				m_sEncodeData.assign(pData, sizeof(PKG_HEAD));
				if ( dwDataSize < sizeof(PKG_HEAD)+1 )
				{
					return ;
				}
				unsigned char ucEncodeType = *(unsigned char*)(m_sDecodeData.data()+sizeof(PKG_HEAD));
				m_dwPos += sizeof(PKG_HEAD)+1;
				if ( ucEncodeType == EncodeType::NORMAL ){
					//正常编解码
					if ( m_dwPos <  m_sDecodeData.size() )
						m_oAnyValues.decode(m_dwPos, m_sDecodeData.data(), m_sDecodeData.size());
				}
				else
				{
					throw Error("decode error: no suppert encode type.");
					return;
				}
			}

			const char* data()	{	return m_sEncodeData.data();	}
			size_t size()	{	return m_sEncodeData.size();	}
			const char* bodydata()	{	return m_sEncodeData.data()+sizeof(PKG_HEAD);	}
			const size_t bodysize()	{	return m_sEncodeData.size()-sizeof(PKG_HEAD);	}
			void setbodydata(const unsigned char* pData, const size_t dwSize){		this->setbodydata((char*)pData, dwSize);	}
			void setbodydata(const char* pData, const size_t dwSize){		m_sEncodeData.replace(sizeof(PKG_HEAD),std::string::npos,  pData,dwSize);	}
			const CAnyValue& operator[](const CKey& oKey) const
			{
				return m_oAnyValues.find(oKey);
			}

			CAnyValue& operator[](const CKey& oKey)
			{
				return m_oAnyValues.find(oKey);
			}

			bool hasKey(const CKey& oKey) const {
				return m_oAnyValues.hasKey(oKey);
			}	
			void insert(const CKey& oKey, const CAnyValue& oValue)
			{
				m_oAnyValues.insert(oKey, oValue);
			}

			void erase(const CKey& oKey)
			{
				m_oAnyValues.erase(oKey);
			}

			void clear()	{	m_oAnyValues.clear();	m_sDecodeData.assign(sizeof(PKG_HEAD), 0);	}
			void clearbody(){	m_oAnyValues.clear();	m_sDecodeData.erase(sizeof(PKG_HEAD));	}
			CAnyValue& root() {	return m_oAnyValues;	}
			const CAnyValue& root() const {	return m_oAnyValues;	}
			void setroot(const CAnyValue& any)	{	m_oAnyValues = any;	}
		private:
			void assign(const this_type& rhs)
			{
				m_sEncodeData = rhs.m_sEncodeData;
				m_sDecodeData = rhs.m_sDecodeData;
				m_dwPos = rhs.m_dwPos;
			}

		private:
			size_t m_dwPos;
			std::string m_sEncodeData;
			std::string m_sDecodeData;

			CAnyValue m_oAnyValues;
		};




		class CAnyValueRoot{
		public:
			typedef std::runtime_error Error;
			typedef CAnyValueRoot this_type;
		public:
			CAnyValueRoot(){
			}

			CAnyValueRoot(const unsigned char* pszPkgData,const int32_t iLen){
				CAnyValueRoot((char*)pszPkgData, iLen);
			}


			CAnyValueRoot(const char* pszPkgData,const int32_t iLen)
			{
				if (NULL == pszPkgData) 
				{
					throw Error("pszPkgData is null.");
				}
				m_sDecodeData.erase();
				m_sDecodeData.assign((char*)pszPkgData,iLen);
			}

			~CAnyValueRoot(){}

			void encodeJSON(const bool bUtf8=true,const bool bEncodeJS=true){
				m_sEncodeData.clear();
				m_oAnyValues.encodeJSON(m_sEncodeData, bUtf8, bEncodeJS);
			}
			void encodeXML(const bool bUtf8=true, const bool bEncodeJS=true){
				m_sEncodeData.clear();
				if (bUtf8)
					m_sEncodeData += "<?xml version=\"1.0\" encoding=\"UTF-8\"?><anyvalue>";
				else
					m_sEncodeData += "<?xml version=\"1.0\" encoding=\"GB2312\"?><anyvalue>";

				m_oAnyValues.encodeXML(m_sEncodeData, bUtf8, bEncodeJS);
				m_sEncodeData += "</anyvalue>";
			}


			void encode(const unsigned char ucEncodeType=EncodeType::NORMAL){
				//清除包体数据
				m_sEncodeData.clear();
				m_sEncodeData.append((char*)&ucEncodeType, sizeof(ucEncodeType));
				//正常编解码
				m_oAnyValues.encode(m_sEncodeData);
			}

			void decode(const std::string& sData)
			{
				this->decode(sData.data(), sData.size());
			}
			void decode(const unsigned char* pData, const size_t dwDataSize){
				this->decode((char*)pData, dwDataSize);
			}
			void decode(const char* pData, const size_t dwDataSize){
				m_dwPos = 0;
				m_oAnyValues.clear();
				m_sDecodeData.assign(pData, dwDataSize);
				if ( dwDataSize < 1 )
				{
					return ;
				}
				unsigned char ucEncodeType = *(unsigned char*)(m_sDecodeData.data());
				m_dwPos += 1;
				if ( ucEncodeType == EncodeType::NORMAL ){
					//正常编解码
					if ( m_dwPos <  m_sDecodeData.size() )
						m_oAnyValues.decode(m_dwPos, m_sDecodeData.data(), m_sDecodeData.size());
				}
				else
				{
					throw Error("decode error: no suppert encode type.");
					return ;
				}

			}

			const char* data()	{	return m_sEncodeData.data();	}
			size_t size()	{	return m_sEncodeData.size();	}
			const CAnyValue& operator[](const CKey& oKey) const
			{
				return m_oAnyValues.find(oKey);
			}

			CAnyValue& operator[](const CKey& oKey)
			{
				return m_oAnyValues.find(oKey);
			}

			bool hasKey(const CKey& oKey) const {
				return m_oAnyValues.hasKey(oKey);
			}

			void insert(const CKey& oKey, const CAnyValue& oValue)
			{
				m_oAnyValues.insert(oKey, oValue);
			}

			void erase(const CKey& oKey)
			{
				m_oAnyValues.erase(oKey);
			}

			void clear()	{	m_oAnyValues.clear();	m_sDecodeData.clear();	}
			CAnyValue& root() {	return m_oAnyValues;	}
			const CAnyValue& root() const {	return m_oAnyValues;	}
			void setroot(const CAnyValue& any)	{	m_oAnyValues = any;	}
		private:
			void assign(const this_type& rhs)
			{
				m_sEncodeData = rhs.m_sEncodeData;
				m_sDecodeData = rhs.m_sDecodeData;
				m_dwPos = rhs.m_dwPos;
			}

		private:
			size_t m_dwPos;
			std::string m_sEncodeData;
			std::string m_sDecodeData;
			CAnyValue m_oAnyValues;
		};

	}
}

#endif

