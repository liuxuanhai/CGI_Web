#ifndef _LCE_NET_DATAGRAM_STRING_BUFFER_FACTORY_H_
#define _LCE_NET_DATAGRAM_STRING_BUFFER_FACTORY_H_


#include <stdint.h>
#include "net/listbytebuffer.h"
#include "net/idatagramfactory.h"
#include "net/binoarchive.h"
#include "net/biniarchive.h"
#include "datagramstringbuffer.h"

namespace lce
{
	namespace net
	{
		
		class DatagramStringBufferFactory : public IDatagramFactory
		{
		public:
			DatagramStringBufferFactory();
			virtual ~DatagramStringBufferFactory();
		
		public:
			void encode(ListByteBuffer* lbb, IDatagram* d);
			IDatagram* decode(ListByteBuffer* lbb);
			int frame(ListByteBuffer* lbb);
			
		private:
			static uint32_t get_magic(ListByteBuffer* lbb);
			static uint32_t get_length(ListByteBuffer* lbb);
			
		private:
			char _buffer[DatagramStringBuffer::MAX_STRUCT_SIZE];
		};
		
		
		inline
		DatagramStringBufferFactory::DatagramStringBufferFactory()
		{
			new(_buffer)DatagramStringBuffer();
		}
		
		inline
		DatagramStringBufferFactory::~DatagramStringBufferFactory()
		{
			reinterpret_cast<DatagramStringBuffer*>(_buffer)->~DatagramStringBuffer();
		}
		
		inline
		uint32_t DatagramStringBufferFactory::get_magic(ListByteBuffer* lbb)
		{
			const ListBuffer::Position pos_keep = lbb->position();
			lbb->position(pos_keep + DatagramStringBuffer::POS_MAGIC);
			const uint32_t magic = static_cast<uint32_t>(lbb->get_int());
			lbb->position(pos_keep);
			
			return magic;
		}
		
		inline
		uint32_t DatagramStringBufferFactory::get_length(ListByteBuffer* lbb)
		{
			const ListBuffer::Position pos_keep = lbb->position();
			lbb->position(pos_keep + DatagramStringBuffer::POS_LENGTH);
			const uint32_t length = static_cast<uint32_t>(lbb->get_int());
			lbb->position(pos_keep);
			
			return length;
		}
		
		inline
		void DatagramStringBufferFactory::encode(ListByteBuffer* lbb, IDatagram* d)
		{
			ListByteBuffer::Position pos_begin = lbb->position();
			
			BinOArchive<ListByteBuffer> oarchive(*lbb);
			DatagramStringBuffer* datagram = dynamic_cast<DatagramStringBuffer*>(d);

			oarchive & (*datagram);
		
			const ListBuffer::Position pos_end = lbb->position();
		
			// ÉèÖÃ³¤¶È×Ö¶Î
			lbb->position(pos_begin + DatagramStringBuffer::POS_LENGTH);
			lbb->put_int(pos_end - pos_begin);
			lbb->position(pos_end);
		}
		
		inline
		IDatagram* DatagramStringBufferFactory::decode(ListByteBuffer* lbb)
		{
			BinIArchive<ListByteBuffer> iarchive(*lbb);
			reinterpret_cast<IDatagram*>(_buffer)->~IDatagram();
			DatagramStringBuffer* d = new(_buffer)DatagramStringBuffer();
			iarchive & (*d);
			return d;
		}
		
		inline
		int DatagramStringBufferFactory::frame(ListByteBuffer* lbb)
		{
			if(lbb->remaining() < static_cast<int>(DatagramStringBuffer::HEADER_LENGTH))
			{
				return DatagramStringBuffer::HEADER_LENGTH;
			}
		
			const ListBuffer::Position pos_keep = lbb->position();
			int expected_length = -1;
		
			if (get_magic(lbb) == DatagramStringBuffer::DATAGRAM_STRING_BUFFER_MAGIC)
			{
				expected_length = get_length(lbb);
		
				// check length valid
				if((expected_length < static_cast<int>(DatagramStringBuffer::HEADER_LENGTH)) ||
				   (expected_length > static_cast<int>(DatagramStringBuffer::MAX_LENGTH)))
				{
						expected_length = -1;
				}
			}
			else // invalid datagram
			{
				expected_length = -1;
			}
		
			lbb->position(pos_keep);
			return expected_length;
		}
	}
}

#endif

