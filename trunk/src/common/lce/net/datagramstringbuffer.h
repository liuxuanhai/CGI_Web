#ifndef _LCE_NET_DATAGRAM_STRING_BUFFER_H_
#define _LCE_NET_DATAGRAM_STRING_BUFFER_H_

#include <stdint.h>
#include "net/idatagram.h"

using namespace std;
namespace lce
{
	namespace net
	{
		class DatagramStringBuffer: public IDatagram
		{
		public:
		    //协议信息
		    static const uint32_t DATAGRAM_STRING_BUFFER_MAGIC = 0x1f24ff88U;
		    static const uint32_t MAX_LENGTH = 32 * 1024 * 1024; // 32M
		    static const uint32_t MAX_STRUCT_SIZE = 8 * 1024 * 1024; // 8M
		    static const uint32_t HEADER_LENGTH = 8;

		    //成员偏移信息
		    static const uint32_t POS_MAGIC = 0;
		    static const uint32_t POS_LENGTH = 4;
		    static const uint32_t POS_STRING_BUFFER = 8;

		    
		public:
		    DatagramStringBuffer()
		            : _magic(DATAGRAM_STRING_BUFFER_MAGIC)
		            , _length(0)
		    {
		    }
		    
		    virtual ~DatagramStringBuffer(){}

			//实现IDatagram接口， 不做实际使用
			virtual uint8_t get_datagram_type()
			{
				assert(false);
				return -1;
			}

		    //getters
		    uint32_t get_magic() const
		    {
		        return _magic;
		    }

		    uint32_t get_length() const
		    {
		        return _length;
		    }

		    template<class Archive>
		    inline
		    void serialize(Archive &ar)
		    {
		        ar & _magic;
		        ar & _length;
		        ar & _strbuffer;
		    }

		public:
		    uint32_t _magic;
		    uint32_t _length;
		    string _strbuffer;
		};
	}
}

#endif

