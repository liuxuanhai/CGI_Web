#ifndef _LCE_DATAGRAMCHANNEL_H_
#define _LCE_DATAGRAMCHANNEL_H_

#include "net/socketaddress.h"
#include "net/selectablechannel.h"
#include "net/socketchannel.h"
#include "net/bytebuffer.h"
#include "net/bytechannel.h"

namespace lce
{
    namespace net
    {
        class DatagramChannel : public SelectableChannel, public ByteChannel
        {
        private:
            int _fd;

            // State, increases monotonically

            static  int ST_UNINITIALIZED;
            static  int ST_UNCONNECTED;
            static  int ST_CONNECTED;
            static  int ST_KILLED;

            int _state;

            // Binding
            InetSocketAddress _local_address;
            InetSocketAddress _remote_address;

            // Options, created on demand
            //SocketOpts.IP.TCP options;


        public:
            // Constructor for normal connecting sockets
            //
            DatagramChannel();

            ~DatagramChannel();
           
            /**
             * Opens a socket channel.
             *
             * <p> The new channel is created by invoking the {@link
             * java.nio.channels.spi.SelectorProvider#openSocketChannel
             * openSocketChannel} method of the system-wide default {@link
             * java.nio.channels.spi.SelectorProvider} object.  </p>
             *
             * @return  A new socket channel
             *
             * @throws  IOException
             *          If an I/O error occurs
             */
            void open();
     
            /**
             * Returns an operation set identifying this channel's supported
             * operations.
             *
             * <p> Socket channels support connecting, reading, and writing, so this
             * method returns <tt>(</tt>{@link SelectionKey#OP_CONNECT}
             * <tt>|</tt>&nbsp;{@link SelectionKey#OP_READ} <tt>|</tt>&nbsp;{@link
             * SelectionKey#OP_WRITE}<tt>)</tt>.  </p>
             *
             * @return  The valid-operation set
             */
            int valid_ops();
            bool ensureOpen();

            int send(ByteBuffer& src, InetSocketAddress& target);
            int receive(ByteBuffer& dst, InetSocketAddress& sa_in);
            /**
             * @throws  NotYetConnectedException
             *          If this channel is not yet connected
             */
            int read(ByteBuffer& bb);

            int read(ListByteBuffer& lbb);
            
            /**
             * @throws  NotYetConnectedException
             *          If this channel is not yet connected
             */
            //long read(ByteBuffer[] dsts, int offset, int length);

            /**
             * @throws  NotYetConnectedException
             *          If this channel is not yet connected
             */
            //long read(ByteBuffer[] dsts);

            /**
             * @throws  NotYetConnectedException
             *          If this channel is not yet connected
             */
            int write(ByteBuffer& src);

            /**
             * @throws  NotYetConnectedException
             *          If this channel is not yet connected
             */
            // long write(ByteBuffer[] srcs, int offset, int length);

            /**
             * @throws  NotYetConnectedException
             *          If this channel is not yet connected
             */
            //long write(ByteBuffer[] srcs);

            //SocketOpts options();

            bool is_bound() ;
            InetSocketAddress local_address();
            InetSocketAddress remote_address();
            
            int bind(InetSocketAddress& local) ;

            bool is_connected();
            void ensure_open_and_unconnected();
            bool connect(InetSocketAddress& sa);
            DatagramChannel& disconnect();


            // AbstractInterruptibleChannel synchronizes invocations of this method
            // using AbstractInterruptibleChannel.closeLock, and also ensures that this
            // method is only ever invoked once.  Before we get to this method, is_open
            // (which is volatile) will have been set to false.
            //
            void impl_close_selectablechannel();
            void close();
            void kill();

            /**
             * Translates native poll revent ops into a ready operation ops
             */
            bool translate_ready_ops(int ops, int initialOps, SelectionKey& sk) ;
            bool translate_and_update_ready_ops(int ops, SelectionKey& sk);
            bool translate_and_set_ready_ops(int ops, SelectionKey& sk);
            bool translate_and_set_interest_ops(int ops, SelectionKey& sk);

            int get_fd();

        private:
            //long read0(ByteBuffer[] bufs);
            //long write0(ByteBuffer[] bufs) ;
            void impl_configure_blocking(bool block);
        };
    }
}

#endif

