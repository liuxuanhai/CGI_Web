#include <errno.h>
#include <assert.h>
#include <sys/poll.h>
#include "net/net.h"
#include "net/selector.h"
#include "net/selectionkey.h"
#include "net/datagramchannel.h"

namespace lce
{
    namespace net
    {
        int  DatagramChannel::ST_UNINITIALIZED = -1;
        int  DatagramChannel::ST_UNCONNECTED = 0;

        int  DatagramChannel::ST_CONNECTED = 1;
        int  DatagramChannel::ST_KILLED = 2;


        DatagramChannel::DatagramChannel()
        {
            _state = ST_UNINITIALIZED;
        }

        DatagramChannel::~DatagramChannel()
        {
            close();
        }
        

        /**
         * Opens a socket channel.
         *
         * <p> The new channel is created by invoking the {@link
         * java.nio.channels.spi.SelectorProvider#openDatagramChannel
         * openDatagramChannel} method of the system-wide default {@link
         * java.nio.channels.spi.SelectorProvider} object.  </p>
         *
         * @return  A new socket channel
         *
         * @throws  IOException
         *          If an I/O error occurs
         */
        void DatagramChannel::open()
        {
            SelectableChannel::open();

            _state = ST_UNINITIALIZED;

            _fd = Net::socket(false);

            _state = ST_UNCONNECTED;
        }

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
        int DatagramChannel::valid_ops() {
            return (SelectionKey::OP_READ
                    | SelectionKey::OP_WRITE
                    | SelectionKey::OP_CONNECT);
        }




        bool DatagramChannel::ensureOpen()
        {
            if (!is_open())
                return false;//throw new ClosedChannelException();
            return true;
        }


        /**
         * @throws  NotYetConnectedException
         *          If this channel is not yet connected
         */



        int DatagramChannel::read(ByteBuffer& bb)
        {
            if (!ensureOpen())
            {
                return -1;
            }

            if (!is_open())
            {
                return -1;
            }

            for (;;)
            {
                int pos = bb.position();
                int lim = bb.limit();

                int rem = (pos <= lim ? lim - pos : 0);

                if (rem == 0)
                {
                    return -1;
                }

                int n = 0;

                n = ::read(_fd, bb.address() + pos, rem);

                if (n > 0)
                {
                    bb.position(pos + n);
                }
                return n;
            }

        }

        int DatagramChannel::read(ListByteBuffer& bb)
        {
            return -1;
        }



        /*
            long DatagramChannel::read(ByteBuffer[] dsts) {
            return read(dsts, 0, dsts.length);
            }
            long DatagramChannel::read0(ByteBuffer[] bufs)
            {

                if (bufs == null)
                    throw new NullPointerException();
            synchronized (readLock) {
                    if (!ensure_read_open())
                        return -1;
                long n = 0;
                try {
                begin();
                if (!is_open())
                    return 0;
                readerThread = NativeThread.current();
                for (;;) {
                    n = IOUtil.read(_fd, bufs, nd);
                    if ((n == IOStatus.INTERRUPTED) && is_open())
                    continue;
                    return IOStatus.normalize(n);
                }
                } finally {
                readerThread = 0;
                end(n > 0 || (n == IOStatus.UNAVAILABLE));
                synchronized (stateLock) {
                    if ((n <= 0) && (!is_input_open))
                    return IOStatus.EOF;
                }
                assert IOStatus.check(n);
                }
            }




            }

            long DatagramChannel::read(ByteBuffer[] dsts, int offset, int length)
              {

                if ((offset < 0) || (length < 0) || (offset > dsts.length - length))
                    throw new IndexOutOfBoundsException();
            // ## Fix IOUtil.write so that we can avoid this array copy
            return read0(Util.subsequence(dsts, offset, length));

                     //////

            }
        */

        int DatagramChannel::write(ByteBuffer& bb)
        {
            ensureOpen();

            if (!is_open())
            {
                return -1;
            }

            for (;;)
            {
                int pos = bb.position();
                int lim = bb.limit();
                assert (pos <= lim);
                int rem = (pos <= lim ? lim - pos : 0);

                int written = 0;
                if (rem == 0)
                {
                    return -1;
                }

                written = ::write(_fd, bb.address() + pos, rem);

                if (written > 0)
                {
                    bb.position(pos + written);
                }
                return written;
            }

        }

        int DatagramChannel::send(ByteBuffer& src, InetSocketAddress& target)
        {
            ensureOpen();

            if (!is_open())
            {
                return -1;
            }

            for (;;)
            {
                int pos = src.position();
                int lim = src.limit();
                assert (pos <= lim);
                int rem = (pos <= lim ? lim - pos : 0);

                int written = 0;
                if (rem == 0)
                {
                    return -1;
                }

                struct sockaddr sa;
                unsigned int sa_len = sizeof(struct sockaddr_in);
                Net::InetAddressToSockaddr(target.get_address(),  target.get_port(), &sa, (int*)&sa_len);
                written = ::sendto(_fd, src.address() + pos, rem, 0, &sa, sa_len);

                if (written > 0)
                {
                    src.position(pos + written);
                }
                return written;
            }
        }
        int  DatagramChannel::receive(ByteBuffer& dst, InetSocketAddress& sa_in)
        {
            if (!ensureOpen())
            {
                return -1;
            }

            if (!is_open())
            {
                return -1;
            }

            struct sockaddr sa;
            unsigned int sa_len = sizeof(struct sockaddr_in);

            for (;;)
            {
                int pos = dst.position();
                int lim = dst.limit();

                int rem = (pos <= lim ? lim - pos : 0);

                if (rem == 0)
                {
                    return -1;
                }

                int n = 0;

                n = ::recvfrom(_fd, dst.address() + pos, rem, 0, &sa, (socklen_t*)&sa_len);

                if (n > 0)
                {
                    dst.position(pos + n);
                    
                    sockaddr_in* sin = reinterpret_cast<sockaddr_in*>(&sa);
                    sa_in = InetSocketAddress(ntohl(sin->sin_addr.s_addr), ntohs(sin->sin_port));
                }
                
                return n;
            }

        }


        /*
            long DatagramChannel::write(ByteBuffer[] srcs) {
            return write(srcs, 0, srcs.length);
            }
            long DatagramChannel::write0(ByteBuffer[] bufs)
            {
                if (bufs == null)
                    throw new NullPointerException();
            synchronized (writeLock) {
                    ensure_write_open();
                long n = 0;
                try {
                begin();
                if (!is_open())
                    return 0;
                writerThread = NativeThread.current();
                for (;;) {
                    n = IOUtil.write(_fd, bufs, nd);
                    if ((n == IOStatus.INTERRUPTED) && is_open())
                    continue;
                    return IOStatus.normalize(n);
                }
                } finally {
                writerThread = 0;
                end((n > 0) || (n == IOStatus.UNAVAILABLE));
                synchronized (stateLock) {
                    if ((n <= 0) && (!is_onput_open))
                    throw new AsynchronousCloseException();
                }
                assert IOStatus.check(n);
                }
            }
            }

            long DatagramChannel::write(ByteBuffer[] srcs, int offset, int length)
            {
                if ((offset < 0) || (length < 0) || (offset > srcs.length - length))
                    throw new IndexOutOfBoundsException();
            // ## Fix IOUtil.write so that we can avoid this array copy
            return write0(Util.subsequence(srcs, offset, length));
            }

        */
        void DatagramChannel::impl_configure_blocking(bool block)
        {
            Net::configure_blocking(_fd, block);
        }
        /*
            SocketOpts DatagramChannel::options()
            {
            synchronized (stateLock) {
                if (options == null) {
                SocketOptsImpl.Dispatcher d
                    = new SocketOptsImpl.Dispatcher() {
                        int getInt(int opt) throws IOException {
                        return Net.getIntOption(_fd, opt);
                        }
                        void setInt(int opt, int arg)
                        throws IOException
                        {
                        Net.setIntOption(_fd, opt, arg);
                        }
                    };
                options = new SocketOptsImpl.IP.TCP(d);
                }
                return options;
            }
            }
        */
        bool DatagramChannel::is_bound()
        {
            //return Net.localPortNumber(_fd) != 0;
            return false;
        }

        InetSocketAddress DatagramChannel::local_address()
        {
            if ((_state == ST_CONNECTED) && (!_local_address.is_valid())) {
                // Socket was not bound before connecting,
                // so ask what the address turned out to be
                _local_address = Net::local_address(_fd);
            }
            return _local_address;
        }

        InetSocketAddress DatagramChannel::remote_address()
        {
            return _remote_address;
        }

        int  DatagramChannel::bind(InetSocketAddress& local)
        {
            ensureOpen();
            if (_local_address.is_valid())
                throw   AlreadyBoundException();

            Net::bind(_fd, local.get_address(), local.get_port());
            _local_address = local;

            return 0;
        }

        bool DatagramChannel::is_connected()
        {
            return (_state == ST_CONNECTED);
        }


        void DatagramChannel::ensure_open_and_unconnected()
        {
            if (!is_open())
                return ;//throw  new ClosedChannelException;
            if (_state == ST_CONNECTED)
                return ;//throw  new AlreadyConnectedException;
        }

        bool DatagramChannel::connect(InetSocketAddress& sa)
        {
            int trafficClass = 0;       // ## Pick up from options
            //int localPort = 0;


            ensure_open_and_unconnected();

            int n = 0;

            if (is_open())
            {

                for (;;)
                {
                    n = Net::connect(_fd,
                                     sa.get_address(),
                                     sa.get_port(),
                                     trafficClass);
                    if (  ((n == -1)&&(errno == EINTR))
                            && is_open())
                        continue;
                    break;
                }
            }


            _remote_address = sa;
            if (n == 0)
            {
                // Connection succeeded; disallow further
                // invocation
                _state = ST_CONNECTED;
                return true;
            }
            return false;

        }

    /*
        // AbstractInterruptibleChannel synchronizes invocations of this method
        // using AbstractInterruptibleChannel.closeLock, and also ensures that this
        // method is only ever invoked once.  Before we get to this method, is_open
        // (which is volatile) will have been set to false.
        //
        void DatagramChannel::impl_close_selectablechannel()
        {
            // If this channel is not registered then it's safe to close the fd
            // immediately since we know at this point that no thread is
            // blocked in an I/O operation upon the channel and, since the
            // channel is marked closed, no thread will start another such
            // operation.  If this channel is registered then we don't close
            // the fd since it might be in use by a selector.  In that case
            // closing this channel caused its keys to be cancelled, so the
            // last selector to deregister a key for this channel will invoke
            // kill() to close the fd.
            //
            if (!is_registered())
                kill();
        }
    */
        void DatagramChannel::close()
        {
            deregister();

            if (!is_registered())
            {
                kill();
            }

            SelectableChannel::close();
        }
        
        void DatagramChannel::kill()
        {
            if (_state == ST_KILLED)
                return;
            if (_state == ST_UNINITIALIZED)
            {
                _state = ST_KILLED;
                return;
            }

            Net::close(_fd);
            _state = ST_KILLED;
        }

        /**
         * Translates native poll revent ops into a ready operation ops
         */
        bool DatagramChannel::translate_ready_ops(int ops, int initialOps,SelectionKey& sk)
        {
            int intOps = sk.nio_interest_ops(); // Do this just once, it synchronizes
            int oldOps = sk.nio_ready_ops();
            int newOps = initialOps;

            if ((ops & Selector::OP_NVAL) != 0) {
                // This should only happen if this channel is pre-closed while a
                // selection operation is in progress
                // ## Throw an error if this channel has not been pre-closed
                return false;
            }

            if ((ops & (Selector::OP_ERR | Selector::OP_HUP)) != 0) {
                newOps = intOps;
                sk.nio_ready_ops(newOps);
                // No need to poll again in check_connect,
                // the error will be detected there

                return (newOps & ~oldOps) != 0;
            }

            if (((ops & Selector::OP_READ) != 0) &&
                    ((intOps & SelectionKey::OP_READ) != 0) )
                newOps |= SelectionKey::OP_READ;


            if (((ops & Selector::OP_WRITE) != 0) &&
                    ((intOps & SelectionKey::OP_WRITE) != 0) )
                newOps |= SelectionKey::OP_WRITE;

            sk.nio_ready_ops(newOps);
            return (newOps & ~oldOps) != 0;
        }

        bool DatagramChannel::translate_and_update_ready_ops(int ops, SelectionKey& sk)
        {
            return translate_ready_ops(ops, sk.ready_ops(), sk);
        }

        bool DatagramChannel::translate_and_set_ready_ops(int ops, SelectionKey& sk)
        {
            return translate_ready_ops(ops, 0, sk);
        }

        /**
         * Translates an interest operation set into a native poll event set
         */
        bool  DatagramChannel::translate_and_set_interest_ops(int ops, SelectionKey& sk)
        {
            int newOps = 0;
            if ((ops & SelectionKey::OP_READ) != 0)
                newOps |= Selector::OP_READ;
            if ((ops & SelectionKey::OP_WRITE) != 0)
                newOps |= Selector::OP_WRITE;
            if ((ops & SelectionKey::OP_CONNECT) != 0)
                newOps |= Selector::OP_READ;
            sk.selector()->put_event_ops(sk, newOps);
            return true;
        }

        int DatagramChannel::get_fd()
        {
            return _fd;
        }
    
    }
    
}



