#ifndef _LCE_CGI_VALUE_H_
#define _LCE_CGI_VALUE_H_

#include <string>
#include <stdlib.h>
#include "cgi_utils.h"

namespace lce
{
	namespace cgi
	{
		class CCgiValue
		{
		public:

			CCgiValue(const std::string& sValue)
				:m_sValue(sValue)
			{}
			CCgiValue(const char* pValue)
				:m_sValue(pValue)
			{}
			CCgiValue(const CCgiValue& rhs)
				:m_sValue(rhs.m_sValue)
			{}
			CCgiValue(const int16_t _value)
				:m_sValue(ToStr(_value))
			{}
			CCgiValue(const uint16_t _value)
				:m_sValue(ToStr(_value))
			{}
			CCgiValue(const int32_t _value)
				:m_sValue(ToStr(_value))
			{}
			CCgiValue(const uint32_t _value)
				:m_sValue(ToStr(_value))
			{}	
			CCgiValue(const int8_t _value)
				:m_sValue(ToStr(_value))
			{}
			CCgiValue(const uint8_t _value)
				:m_sValue(ToStr(_value))
			{}	
			CCgiValue(const uint64_t _value)
				:m_sValue(ToStr(_value))
			{}	
			CCgiValue(const int64_t _value)
				:m_sValue(ToStr(_value))
			{}			
			
		#ifdef __x86_64__
		#elif __i386__		
			CCgiValue(const unsigned long _value)
				:m_sValue(ToStr(_value))
			{}
			CCgiValue(const long _value)
				:m_sValue(ToStr(_value))
			{}
		#endif
				
			CCgiValue& operator=(const CCgiValue& rhs){
				if ( this != &rhs )
				{
					m_sValue = rhs.m_sValue;
				}
				return *this;
			}

			inline int64_t asInt() const {		return atoll(m_sValue.c_str());	}
			inline const std::string& asString() const {		return m_sValue;	}
			
			inline operator uint64_t() const {	return atoll(m_sValue.c_str());	}
			inline operator int64_t() const {	return atoll(m_sValue.c_str());	}
			inline operator unsigned char() const {	return static_cast<unsigned char>(asInt());	}
			inline operator char() const {		return static_cast<char>(asInt());	}
			inline operator uint16_t() const {	return static_cast<uint16_t>(asInt());	}
			inline operator int16_t() const {	return static_cast<int16_t>(asInt());	}

		#ifdef __x86_64__
		#elif __i386__	
			inline operator unsigned long() const {	return static_cast<unsigned long>(asInt());	}
			inline operator long() const {	return static_cast<long>(asInt());	}
		#endif
			
			inline operator int32_t() const {	return static_cast<int32_t>(asInt());	}
			inline operator uint32_t() const {	return static_cast<uint32_t>(asInt());	}
			inline operator bool() const {	return asInt()==0 ? false : true;	}
			inline operator std::string() const {	return m_sValue;	}

			const char* data() const {	return m_sValue.data();	}
			const size_t size() const {	return m_sValue.size();	}

		private:
			std::string m_sValue;
		};
	}	
}


#endif

