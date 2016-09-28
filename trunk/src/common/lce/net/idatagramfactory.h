#ifndef _LCE_IDATAGRAMFACTORY_H_
#define _LCE_IDATAGRAMFACTORY_H_

namespace lce
{
	namespace net
	{
		class ListByteBuffer;
		class IDatagram;
		class IDatagramFactory
		{
		public:

			// IDatagramFactory(){}
			virtual ~IDatagramFactory(){}
			
			/**
			 * Reads and processes bytes from a buffer and produces an instance
			 * of IDatagram that can be passed to datagram processing logic.
			 * <P>
			 * The byte buffer that is passed into this method is a subset
			 * of the buffer passed into the frame() method call immediately preceding,
			 * and the buffer begins with the header information that may have been
			 * processed by frame.
			 * <P>
			 * @param bb a byte buffer with the position set to zero and the
			 * limit set to the number of bytes in the buffer,
			 * @return the datagram object encoded in the byte stream
			 * @throws IllegalArgumentException if the byte buffer does not contain
			 *  a valid datagram.
			 */
		    virtual IDatagram* decode( ListByteBuffer* lbb ) = 0;

			virtual bool need_raw_datagram()
			{
				return false;
			}
			
			/**
			 * Provides information to the I/O processing thread about a
			 * raw incoming data stream. The byte buffer passed as a parameter
			 * could potentially be the actual read buffer owned by the I/O thread.
			 * It is critical that this method not change the limit, or
			 * capacity of the buffer.  <P>
			 *
			 * when this method returns, the byte buffer's position should be set to the
			 * origin of the next datagram. the frame logic is allowed to skip arbitrary
			 * bytes in the input stream that are deemed invalid or extraneous.
			 *
			 * the caller of this method will call incoming() with an appropriate, well-formed
			 * byte buffer when <code>length</code> bytes are available starting at
			 * the position set when this call returns.
			 *
			 * @return int the length of the datagram, in terms of the buffer passed as a parameter.
			 * If this value is greater than the number of bytes remaining in the buffer,
			 * or is equal to Integer.MAX_VALUE, a valid datagram is assumed not to exist
			 * in the remaining byte stream.
			 * @throws IOException if the buffer is hopelessly unintelligible
			 * and the connection/buffer should be terminated.
			 */
			virtual int frame( ListByteBuffer* lbb ) = 0;
			
			/**
			 * Outputs a datagram to the specified write buffer. The position of the
			 * write buffer should be zero on input. When the call returns, the position
			 * of the write buffer should be after the last byte of data to be
			 * output to the connection.
			 *<P>
			 * @param bb a byte buffer, with position 0 and limit equal to the maximum
			 * amount of data writeable in a single transaction.
			 * @param d the datagram that will be written to the buffer following this
			 * header.
			 */
			virtual void encode( ListByteBuffer* lbb, IDatagram* d ) = 0;	
		};
	}
}
#endif //ifndef

