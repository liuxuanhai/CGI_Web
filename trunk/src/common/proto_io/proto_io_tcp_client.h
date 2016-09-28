#ifndef _COMMON_PROTO_IO_TCP_CLIENT_H_
#define _COMMON_PROTO_IO_TCP_CLIENT_H_

#include <string>
#include "net/listbytebuffer.h"
#include "net/socketchannel.h"
#include "memory/fixed_size_allocator.h"
#include "net/datagramstringbuffer_factory.h"
#include "msg.pb.h"

namespace common
{
	namespace protoio
	{
		class ProtoIOTcpClient
		{
		public:
			ProtoIOTcpClient(const string& strServerIP, const int iPort);
			~ProtoIOTcpClient();
			
		public:
			int io(const ::hoosho::msg::Msg& stRequest, ::hoosho::msg::Msg& stResponse, std::string& strErrMsg, int iTimeoutMS=5000);
			int notify(const ::hoosho::msg::Msg& stRequest, std::string& strErrMsg, int iTimeoutMS=5000);
			
		private:
			int tcp_connect(std::string& strErrMsg, int iTimeoutMS=5000);
			int tcp_send(lce::net::ListByteBuffer& lbb, int iExpectSendSize, std::string& strErrMsg, int iTimeoutMS=5000);
			int tcp_recv(lce::net::ListByteBuffer& lbb, std::string& strErrMsg, int iTimeoutMS=5000);
			int tcp_close();
			int tcp_io(lce::net::ListByteBuffer& lbb, const int send_length, lce::net::ListByteBuffer& lbb_rcv, std::string& strErrMsg, int iTimeoutMS=5000);
			int tcp_notify(lce::net::ListByteBuffer& lbb, const int send_length, std::string& strErrMsg, int iTimeoutMS=5000);
			
			
		private:
			const string m_server_ip;
			const int m_server_port;
			lce::net::SocketChannel* m_sc;
			static lce::memory::FixedSizeAllocator* g_lbb_buf_allocator;
			static lce::net::DatagramStringBufferFactory* g_datagram_stringbuffer_factory;
		};
	}
}

#endif

