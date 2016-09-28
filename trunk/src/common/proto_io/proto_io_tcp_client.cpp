#include "proto_io_tcp_client.h"
#include "util/str_op.h"
#include "net/datagramstringbuffer.h"

namespace common
{
	namespace protoio
	{
		lce::memory::FixedSizeAllocator* ProtoIOTcpClient::g_lbb_buf_allocator = NULL;
		lce::net::DatagramStringBufferFactory* ProtoIOTcpClient::g_datagram_stringbuffer_factory = NULL;
		ProtoIOTcpClient::ProtoIOTcpClient(const string & strServerIP, const int iPort)
			:m_server_ip(strServerIP)
			, m_server_port(iPort)
			, m_sc(NULL)
		{
			
		}

		ProtoIOTcpClient::~ProtoIOTcpClient()
		{
			if(m_sc)
			{
				delete m_sc;
				m_sc = NULL;
			}
		}

		int ProtoIOTcpClient::io(const ::hoosho::msg::Msg& stRequest, ::hoosho::msg::Msg& stResponse, std::string& strErrMsg, int iTimeoutMS)
		{
			if(NULL == g_lbb_buf_allocator)
			{
				g_lbb_buf_allocator = new lce::memory::FixedSizeAllocator(1024*4, 1024*4, 4*1024*1024);
				if(g_lbb_buf_allocator->init()<0)
				{
					g_lbb_buf_allocator = NULL;
					strErrMsg = "lbb_allocator init failed";
					return -1;
				}
			}

			if(NULL == g_datagram_stringbuffer_factory)
			{
				g_datagram_stringbuffer_factory = new lce::net::DatagramStringBufferFactory();
			}
		
			std::string strInnterErrMsg = "";
			//encode
			lce::net::DatagramStringBuffer datagramRequest;
		    if(!stRequest.SerializeToString(&datagramRequest._strbuffer))
		    {
		        strErrMsg = "stRequest.SerializeToString failed";
		        return -1;
		    }
			lce::net::ListByteBuffer lbbRequest(g_lbb_buf_allocator);
			lce::net::ListBuffer::Position posBegin = lbbRequest.position();
			g_datagram_stringbuffer_factory->encode(&lbbRequest, &datagramRequest);
			lce::net::ListBuffer::Position posEnd = lbbRequest.position();
			const int iSendLength = posEnd - posBegin;

			//io
			lce::net::ListByteBuffer lbbResponse(g_lbb_buf_allocator);
			if(tcp_io(lbbRequest, iSendLength, lbbResponse, strInnterErrMsg , iTimeoutMS) < 0)
			{
				strErrMsg = "tcp_io failed, " + strInnterErrMsg;
				return -1;
			}

			//decode
			lbbResponse.flip();
			lce::net::IDatagram* datagram = g_datagram_stringbuffer_factory->decode(&lbbResponse);
			lce::net::DatagramStringBuffer* pDatagramResponse = dynamic_cast<lce::net::DatagramStringBuffer*>(datagram);
			if(!stResponse.ParseFromString(pDatagramResponse->_strbuffer))
			{
				strErrMsg = "stResponse.ParseFromString failed, strbuffer.size=" + lce::util::StringOP::TypeToStr(pDatagramResponse->_strbuffer.size());
				return -1;
			}
				
		    return 0;
		}

		int ProtoIOTcpClient::notify(const ::hoosho::msg::Msg& stRequest, std::string& strErrMsg, int iTimeoutMS)
		{
			if(NULL == g_lbb_buf_allocator)
			{
				g_lbb_buf_allocator = new lce::memory::FixedSizeAllocator(1024*4, 1024*4, 4*1024*1024);
				if(g_lbb_buf_allocator->init()<0)
				{
					g_lbb_buf_allocator = NULL;
					strErrMsg = "lbb_allocator init failed";
					return -1;
				}
			}

			if(NULL == g_datagram_stringbuffer_factory)
			{
				g_datagram_stringbuffer_factory = new lce::net::DatagramStringBufferFactory();
			}
		
			std::string strInnterErrMsg = "";
			//encode
			lce::net::DatagramStringBuffer datagramRequest;
		    if(!stRequest.SerializeToString(&datagramRequest._strbuffer))
		    {
		        strErrMsg = "stRequest.SerializeToString failed";
		        return -1;
		    }
			lce::net::ListByteBuffer lbbRequest(g_lbb_buf_allocator);
			lce::net::ListBuffer::Position posBegin = lbbRequest.position();
			g_datagram_stringbuffer_factory->encode(&lbbRequest, &datagramRequest);
			lce::net::ListBuffer::Position posEnd = lbbRequest.position();
			const int iSendLength = posEnd - posBegin;

			//io
			if(tcp_notify(lbbRequest, iSendLength, strInnterErrMsg , iTimeoutMS) < 0)
			{
				strErrMsg = "tcp_notify failed, " + strInnterErrMsg;
				return -1;
			}
				
		    return 0;
		}
				
		int ProtoIOTcpClient::tcp_connect(std::string& strErrMsg, int iTimeoutMS)
		{
			//first , or reconnect
			if (NULL == m_sc)
			{
				m_sc = new lce::net::SocketChannel();
				m_sc->open();
				if (!m_sc->connect(const_cast<char *>(m_server_ip.c_str()), m_server_port, iTimeoutMS))
				{
					strErrMsg = "lce::net::socketchannel connect failed";
					tcp_close();
					return -1;
				}

				return 0;
			}

			//repeat connect , succ , return directly
			if (m_sc->is_connected())
			{
				return 0;
			}
			
			delete m_sc;
			m_sc = NULL;
			return tcp_connect(strErrMsg, iTimeoutMS);
		}

		int ProtoIOTcpClient::tcp_send(lce::net::ListByteBuffer& lbb, int iExpectSendSize, std::string& strErrMsg, int iTimeoutMS)
		{
			lbb.flip();
			int iRealSendLength = 0;
			while (iRealSendLength < iExpectSendSize)
			{
				int iRet = m_sc->write(lbb);
				if (iRet < 0)
				{
					strErrMsg = "lce::net::socketchannel write(lbb) failed";
					m_sc->close();
					return -1;
				}
				
				iRealSendLength += iRet;
			}
			
			return iRealSendLength;
		}
		
		int ProtoIOTcpClient::tcp_recv(lce::net::ListByteBuffer& lbb, std::string& strErrMsg, int iTimeoutMS)
		{
			lbb.clear();
	
			// recv head.
			char aczBuf[lce::net::DatagramStringBuffer::HEADER_LENGTH] = {0};
			int iReadLength = m_sc->sync_read(&aczBuf[0], lce::net::DatagramStringBuffer::HEADER_LENGTH, iTimeoutMS);
			if (iReadLength <= 0) 
			{
				strErrMsg = "lce::net::socketchannel sync_read head data error. read_length=" + lce::util::StringOP::TypeToStr(iReadLength);
				m_sc->close();
				return -1;
			}
			
		
			lbb.put(aczBuf, lce::net::DatagramStringBuffer::HEADER_LENGTH, lce::net::DatagramStringBuffer::HEADER_LENGTH);
			
			// compute body length.
			const int iExpectTotalLength = ntohl(*((int *)&aczBuf[lce::net::DatagramStringBuffer::POS_LENGTH]));
			const int iExpectBodyLength = iExpectTotalLength - lce::net::DatagramStringBuffer::HEADER_LENGTH;
			if (iExpectBodyLength < 0)
			{
				strErrMsg = "illegal, iExpectBodyLength = " + lce::util::StringOP::TypeToStr(iExpectBodyLength);
				return -1;
			}

			//empty body
			if (0 == iExpectBodyLength)
			{
				return 0;
			}

			// recv rest body.
			char* aczBufBody = new char[iExpectBodyLength];
			::memset(aczBufBody, 0, iExpectBodyLength);
			iReadLength = m_sc->sync_read(aczBufBody, iExpectBodyLength, iTimeoutMS);
			if (iReadLength <= 0) 
			{
				strErrMsg = "lce::net::socketchannel sync_read body data error. read_length=" + lce::util::StringOP::TypeToStr(iReadLength);
				m_sc->close();
				delete aczBufBody;
				return -1;
			}
			
			lbb.put(aczBufBody, iExpectBodyLength, iExpectBodyLength);

			//g_lbb_buf_allocator->deallocate((void *)aczBufBody);
			delete aczBufBody;
			
			return 0;
		}
		
		int ProtoIOTcpClient::tcp_close()
		{
			if (NULL != m_sc)
			{
				m_sc->close();
				delete m_sc;
				m_sc = NULL;
			}

			return 0;
		}
		
		int ProtoIOTcpClient::tcp_io(lce::net::ListByteBuffer& lbb, const int send_length, lce::net::ListByteBuffer& lbb_rcv, std::string& strErrMsg, int iTimeoutMS)
		{
			std::string strInnerErrMsg = "";
			if (tcp_connect(strInnerErrMsg, iTimeoutMS)< 0)
			{   
				strErrMsg = "tcp_connect fail, " + strInnerErrMsg;
				tcp_close();
				return -1;
			}

			if (tcp_send(lbb, send_length, strInnerErrMsg, iTimeoutMS) < 0)
			{   
				strErrMsg = "tcp_send fail, " + strInnerErrMsg;
				tcp_close();
				return -1;
			}   

			if (tcp_recv(lbb_rcv, strInnerErrMsg, iTimeoutMS) < 0)
			{   
				strErrMsg = "tcp_recv fail, " + strInnerErrMsg;
				tcp_close();
				return -1;
			}
			
			return 0;
		}

		int ProtoIOTcpClient::tcp_notify(lce::net::ListByteBuffer& lbb, const int send_length, std::string& strErrMsg, int iTimeoutMS)
		{
			std::string strInnerErrMsg = "";
			if (tcp_connect(strInnerErrMsg, iTimeoutMS)< 0)
			{   
				strErrMsg = "tcp_connect fail, " + strInnerErrMsg;
				tcp_close();
				return -1;
			}

			if (tcp_send(lbb, send_length, strInnerErrMsg, iTimeoutMS) < 0)
			{   
				strErrMsg = "tcp_send fail, " + strInnerErrMsg;
				tcp_close();
				return -1;
			}
			
			return 0;
		}
	}
}



