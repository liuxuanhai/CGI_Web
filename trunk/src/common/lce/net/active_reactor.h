#ifndef _LCE_ACTIVE_REACTOR_H_
#define _LCE_ACTIVE_REACTOR_H_

#include <unistd.h>
#include <ext/hash_map>
#include <string>
#include "memory/array_object_pool.h"
#include "net/selector.h"
#include "net/serversocketchannel.h"
#include "net/socketchannel.h"
#include "net/biniarchive.h"
#include "net/imessageprocessor.h"
#include "net/connectioninfo.h"
#include "net/connectioneventlistener.h"
#include "net/idatagramfactory.h"
#include "net/selectorfactory.h"
#include "memory/fixed_size_allocator.h"
#include "app/timer.h"
#include "net/selectablechannel.h"


using namespace std;
using namespace __gnu_cxx;

namespace __gnu_cxx
{
	template<> 
	struct hash<lce::net::SelectableChannel*>
	{
	    size_t operator()(const lce::net::SelectableChannel* __s) const
	    { 
	    	return  (size_t)__s;  
	    }
	};
}

namespace lce
{
	namespace net
	{
		template< class T = ConnectionInfo >
		class Acceptor;
		
		template< class T = ConnectionInfo >
		class Connector;

		template< class T = ConnectionInfo >
		class ActiveReactor : public ConnectionEventListener, public lce::app::TimerHandler
		{
		    friend class Acceptor<T>;
		    friend class Connector<T>;
		    
		public:
		    struct  ConnectorInfo
		    {
		        IMessageProcessor* _processor;
		        InetSocketAddress   _peer_addr;
		        ConnectionInfo*        _connection_info;
		    };
		        
		    enum
		    {
		     DEFAULT_ACCECPT_ONCE = 128,
		     DEFAULT_BACKLOG = 128,
		     DEFAULT_SELECT_TIMEOUT  = 10,        //10 ms
		     DEFAULT_MAX_CONNECTIONS  = 128,      
		    };

		    typedef lce::memory::ArrayObjectPool<T> ConnectionInfoPool;
		    
		    const uint32_t MAX_ACCEPTOR_NUM;
		    const uint32_t MAX_CONNECTOR_NUM;
			
			ActiveReactor(lce::memory::FixedSizeAllocator* allocator,
				int select_timeout=DEFAULT_SELECT_TIMEOUT,
		        int max_connections = DEFAULT_MAX_CONNECTIONS,
		        int accept_once = DEFAULT_ACCECPT_ONCE,
		        int backlog = DEFAULT_BACKLOG,
		        int max_acceptor_num = 10,
		        int max_connector_num = 30,
		        int reconnect_interval = 3)
			        :MAX_ACCEPTOR_NUM( max_acceptor_num ),
			        MAX_CONNECTOR_NUM( max_connector_num ),
			        _connection_pool(max_connections),
			        _selector(NULL),    
			        _acceptor(NULL),
			        _connector(NULL),
			        _accept_once(accept_once),
			        _backlog(backlog),
			        _select_timeout(select_timeout),  
			        _reconnect_interval(reconnect_interval),
			        _buffer_allocator( allocator )
			{
			        _acceptor = new Acceptor<T>(*this);
			        _connector  = new Connector<T>(*this);
			        
			        //_selector = Selector::open();     
			        _selector = SelectorFactory::factory().open_selector("epoll");
	                _timer_container = new lce::app::TimerContainer(_connection_pool.capacity() + MAX_CONNECTOR_NUM );
		
			}


			~ActiveReactor()
			{
			    if(_acceptor)
			    {
			        delete _acceptor;
			        _acceptor = NULL;
			    }

			    if(_connector)
			    {
			        delete _connector;
			        _connector = NULL;
			    }
				
	            if(_selector)
	            {
	                    delete _selector;
	                    _selector = NULL;
	            }

	            if(_timer_container)
	            {
	                    delete _timer_container;
	                    _timer_container = NULL;
	            }

	            SocketProcessorMap::iterator it = _acceptor_sockets.begin();
	            for( ; it != _acceptor_sockets.end(); )
	            {
	                    ServerSocketChannel* ssc = (ServerSocketChannel*)(it->first);
	                    it ++;
	                    delete ssc;
	            }
			}
		    
		    int add_acceptor(const char* ip, int port, IMessageProcessor& processor, IDatagramFactory& factory );
		    int add_acceptor(int port,  IMessageProcessor& processor, IDatagramFactory& factory );    
		    int add_connector(const char* peer_ip, int peer_port, IMessageProcessor& processor, IDatagramFactory& factory );
		    int async_connect(ConnectorInfo& connector_info);
			int rm_connector(const char* peer_ip, int peer_port);
			int rm_connector(const char* peer_ip, int peer_port, IMessageProcessor& processor);
			//end
		    
		    int init();
		    int poll();
		    template <class Fun>
		    int poll(Fun& f,int select_time_out);
		    void wakeup();
		    void connection_event(ConnectionEvent& e);

			void handle_timeout(void *param);

		private:
		    int accept_pending_connections(SelectionKey& key);
		    int finish_connect(SelectionKey& key);
		    int register_new_channels();
	        size_t rm_connectioninfo_from_unregister_list(const T *ci);

		private:

		    typedef hash_set<IMessageProcessor*>                                 ProcessorSet;
		    typedef hash_map<SelectableChannel*, IMessageProcessor*> SocketProcessorMap;
		    typedef hash_map<SelectableChannel*, ConnectorInfo>         SocketConnectorMap;
			typedef hash_map<SelectableChannel*, IDatagramFactory*> SocketFactoryMap;

			struct StringHash
			{
				size_t operator()(const string& str) const
		        { return __stl_hash_string(str.c_str());  }
			};

			typedef hash_map<string, hash_set<SelectableChannel*>, StringHash> IPSelectableChannelMap;
			IPSelectableChannelMap _ip_connect_channel;

		    ConnectionInfoPool       _connection_pool;    
		    Selector* _selector;

		    ProcessorSet                 _processors;
		    SocketProcessorMap       _acceptor_sockets;
		    SocketConnectorMap       _connector_sockets;
			SocketFactoryMap		 _acceptor_factorys;
		 
		    Acceptor<T>*         _acceptor;
		    Connector<T>*       _connector;
		    
		    ConnectionInfoList _to_register;
		    ConnectionInfoList _to_unregister;

		    int   _accept_once;
		    int   _backlog;
		    int   _select_timeout;
			int   _reconnect_interval;


			lce::memory::FixedSizeAllocator* _buffer_allocator;
			lce::app::TimerContainer* _timer_container;
		};

		
		template< class T >
		void ActiveReactor<T>::wakeup()
		{
		    _selector->wakeup();
		}

		
		template< class T >	
		int ActiveReactor<T>::add_acceptor(int port,  
			IMessageProcessor& processor, IDatagramFactory& factory)
		{
		    return add_acceptor(NULL,  port, processor, factory);
		}


		template< class T >
		class Acceptor : public Attachment
		{
		private:
		    ActiveReactor<T>* _reactor;
		public:
		    Acceptor(ActiveReactor<T>& reactor)
		            : _reactor(&reactor)
		    {
		    }
		    int run(SelectionKey& key);
		};

		
		template< class T >	
		int Acceptor<T>::run(SelectionKey& key)
		{
		    return  _reactor->accept_pending_connections(key);
		}

		template< class T >
		class Connector : public Attachment
		{
		private:
		    ActiveReactor<T>* _reactor;
		public:
		    Connector(ActiveReactor<T>& reactor)
		            : _reactor(&reactor)
		    {
		    }
		    int run(SelectionKey& key);
		};

		
		template< class T >	
		int Connector<T>::run(SelectionKey& key)
		{
		    return  _reactor->finish_connect(key);
		}


		template< class T >  
		int ActiveReactor<T>::add_acceptor(const char* ip, int port, 
			IMessageProcessor& processor, IDatagramFactory& factory)
		{
		    int rv;

		    if(_acceptor_sockets.size() >= MAX_ACCEPTOR_NUM)
		    {
		        return -1;
		    }
		    
		    InetSocketAddress server_addr(ip, port);


		    ServerSocketChannel* ssc =  new ServerSocketChannel;

		    ssc->send_buf_size(1024*1024);
		    ssc->recv_buf_size(1024*1024);

		    rv = ssc->bind(server_addr, _backlog);
		    if (rv < 0)
		    {	delete ssc;
		        return -1;
		    }
		    ssc->configure_blocking(false);

		    //向selector注册该channel
		    SelectionKey* server_sk =  ssc->enregister(*_selector, SelectionKey::OP_ACCEPT);

		    //利用sk的attache功能绑定Acceptor 如果有事情，触发Acceptor

		    server_sk->attach(_acceptor);

		    _acceptor_sockets.insert(SocketProcessorMap::value_type((SelectableChannel*)ssc, &processor));
		    _processors.insert(&processor);
			_acceptor_factorys.insert( SocketFactoryMap::value_type((SelectableChannel*)ssc, &factory));

		    return _acceptor_sockets.size();
		    
		}



		template< class T >
		int ActiveReactor<T>::add_connector(const char* peer_ip, int peer_port,  
			IMessageProcessor& processor, IDatagramFactory& factory)
		{
		    if(_connector_sockets.size() >= MAX_CONNECTOR_NUM)
		    {
		        return -1;
		    }
		    
		    InetSocketAddress	serveraddr(peer_ip, peer_port);

		    ConnectorInfo    connector_info;
		    connector_info._processor = &processor;   
		    connector_info._peer_addr = serveraddr;
		    connector_info._connection_info = dynamic_cast<ConnectionInfo*>(
				_connection_pool.construct(&factory, _buffer_allocator));
		    if(connector_info._connection_info == NULL)
		    {
		        return -1;
		    }    
		    _processors.insert(& processor);

		    SocketChannel* sc = &connector_info._connection_info->get_channel();

		    _connector_sockets.insert(typename SocketConnectorMap::value_type( (SelectableChannel*)sc, connector_info));  

			char tmp[128];
			snprintf( tmp, 128, "%s:%d", peer_ip, peer_port );
			string key( tmp );
			hash_set<SelectableChannel*>& channel_set = _ip_connect_channel[key];
			channel_set.insert( (SelectableChannel*)sc );
		    
		    return async_connect(connector_info);
		}

	        template< class T >
		size_t ActiveReactor<T>::rm_connectioninfo_from_unregister_list(const T *ci)
	        {
	                for (ConnectionInfoList::iterator iter = _to_unregister.begin(); iter != _to_unregister.end(); iter++)
	                {
	                        if((*iter) == ci) // found
	                        {
	                                _to_unregister.erase(iter);
	                                return 1;
	                        }
	                }
	                return 0;
	        }

		template< class T >
		int ActiveReactor<T>::rm_connector(const char* peer_ip, int peer_port)
		{
			char tmp[128];
			snprintf( tmp, 128, "%s:%d", peer_ip, peer_port );
			string key( tmp );

			typename IPSelectableChannelMap::iterator cs_it = _ip_connect_channel.find( key );
			if( cs_it == _ip_connect_channel.end() )
				return 0;
			
			hash_set<SelectableChannel*>& channel_set = cs_it->second;
			hash_set<SelectableChannel*>::iterator it = channel_set.begin();
			for( ; it != channel_set.end(); it ++ )
			{
				typename SocketConnectorMap::iterator iter = _connector_sockets.find(*it);
				if( iter != _connector_sockets.end() )
				{
					if(iter->second._connection_info->is_open())
					{
						iter->second._connection_info->close();
					}
					else
					{
						iter->second._connection_info->close();
	                    T *ci = dynamic_cast<T*>(iter->second._connection_info);
	                    rm_connectioninfo_from_unregister_list(ci);
						_connection_pool.destroy(ci);
					}
	//				_connection_pool.destroy( dynamic_cast<T*>(iter->second._connection_info));
					_connector_sockets.erase( iter );
				}
			}

			_ip_connect_channel.erase( cs_it );

			return 0;
		}

		//add by bing 2009-7-10
		template< class T >
		int ActiveReactor<T>::rm_connector(const char* peer_ip, int peer_port, IMessageProcessor& processor)
		{
			char tmp[128];
			snprintf( tmp, 128, "%s:%d", peer_ip, peer_port );
			string key( tmp );

			typename IPSelectableChannelMap::iterator cs_it = _ip_connect_channel.find( key );
			if( cs_it == _ip_connect_channel.end() )
				return 0;
			
			hash_set<SelectableChannel*>& channel_set = cs_it->second;
			hash_set<SelectableChannel*>::iterator it = channel_set.begin();
			for( ; it != channel_set.end(); it ++ )
			{
				typename SocketConnectorMap::iterator iter = _connector_sockets.find(*it);
				if( iter != _connector_sockets.end() )
				{
					if(iter->second._connection_info->is_open())
					{
						iter->second._connection_info->close();
					}
					else
					{
						iter->second._connection_info->close();
	                    T *ci = dynamic_cast<T*>(iter->second._connection_info);
	                     rm_connectioninfo_from_unregister_list(ci);
						_connection_pool.destroy(ci);
					}

					//do not need destroy
					//_connection_pool.destroy( dynamic_cast<T*>(iter->second._connection_info));
					_connector_sockets.erase( iter );
				}
			}

			_ip_connect_channel.erase( cs_it );

			//need erase the processor
			_processors.erase(&processor);

			return 0;
		}
		//end

		template< class T >
		int ActiveReactor<T>::async_connect(ConnectorInfo& connector_info)
		{
		    connector_info._connection_info->get_channel().open();
		    connector_info._connection_info->add_event_listener(*this);

		    SocketChannel* sc = &connector_info._connection_info->get_channel();
		    sc->configure_blocking(false);
		    sc->send_buf_size(1024*1024);
		    sc->recv_buf_size(1024*1024);    
		    sc->connect(connector_info._peer_addr);
		    //向selector注册该channel
		    if( sc->is_connected() )
	    	{
	    		typename SocketConnectorMap::iterator iter = _connector_sockets.find((SelectableChannel*)sc);    

			    assert(iter != _connector_sockets.end());
				
				sc->tcp_nodelay(true);
				SelectionKey* sk = sc->enregister(*_selector, SelectionKey::OP_READ);
	////可能sk为NULL
				ConnectionInfo* conn = iter->second._connection_info;	
				conn->open(*(iter->second._processor));
				conn->add_event_listener(*this);        

				sk->attach(conn);
	    	}
			else
			{
				SelectionKey* sk  =sc->enregister(*_selector, SelectionKey::OP_CONNECT);	
		    
		    	sk->attach(_connector);
			}
		    
		    //cout<<"async connect send "<<endl;
		    return _connector_sockets.size();    
		}


		template< class T >
		int ActiveReactor<T>::init()
		{
			int rv;
			rv = _selector->init(_connection_pool.capacity()+MAX_ACCEPTOR_NUM+MAX_CONNECTOR_NUM);
			if (rv < 0)
			{
				return -1;
			}
			rv = _connection_pool.init();
			if (rv < 0)
			{
				return -1;
			}

			if( _timer_container->init() < 0 )
			{
				return -1;
			}
			
		    return 0;
		}


		template< class T >
		int ActiveReactor<T>::poll()
		{
			_timer_container->poll();
		    register_new_channels();
			
			int num_keys=0;
			if(_select_timeout==0)
			{
				num_keys = _selector->select_now();
			}
			else
			{
		    	num_keys = _selector->select(_select_timeout);
			}

			
		    //如果有我们注册的事情发生了，它的传回值就会大于0
		    if (num_keys > 0)
		    {
		        SelectionKey* selected_key = NULL;
		        while((selected_key = _selector->next()))
		        {
		            Attachment* r = (Attachment*)(selected_key->attachment());
		            if (r != NULL)
		            {
		                r->run(*selected_key);
		            }
		        }
		    }

		    ProcessorSet::iterator iter = _processors.begin();
		    for(;iter!=_processors.end(); iter++)
		    {
		        (*iter)->process_output();
		    }

		    return num_keys;
		}

		template< class T> template<class Fun>
		int ActiveReactor<T>::poll(Fun& run_functor,int select_time_out)
		{
			_timer_container->poll();
		    register_new_channels();
			
			int num_keys=0;
			if(select_time_out==0)
			{
				num_keys = _selector->select_now();
			}
			else
			{
		    	num_keys = _selector->select(select_time_out);
			}

		    //如果有我们注册的事情发生了，它的传回值就会大于0
		    if (num_keys > 0)
		    {
		        SelectionKey* selected_key = NULL;
		        while((selected_key = _selector->next()))
		        {
		            Attachment* r = (Attachment*)(selected_key->attachment());
		            if (r != NULL)
		            {
			            run_functor(selected_key);
		            }
		        }
		    }

		    ProcessorSet::iterator iter = _processors.begin();
		    for(;iter!=_processors.end(); iter++)
		    {
		        (*iter)->process_output();
		    }

		    return num_keys;
		}

		template< class T >
		int ActiveReactor<T>::accept_pending_connections(SelectionKey& key)
		{
		    int rv =0;
		    int i   = 0;


		    ServerSocketChannel* ready_channel = (ServerSocketChannel*)key.channel();    
		    
		    for (; i < _accept_once; i++)
		    {
		    	SocketFactoryMap::iterator sfiter = _acceptor_factorys.find((SelectableChannel*)ready_channel);
		    	ConnectionInfo* conn = dynamic_cast<ConnectionInfo*>(
					_connection_pool.construct((sfiter->second), _buffer_allocator) );
	            if(conn == NULL)
	            {
	                return i;
	            }
		        rv = ready_channel->accept(conn->get_channel());

		        if (rv == 0)
		        {
		            SocketProcessorMap::iterator iter = _acceptor_sockets.find((SelectableChannel*)ready_channel);	            
		            conn->bind(*(iter->second));
		            _to_register.push_back(conn);
		        }
		        else
		        {
		        	_connection_pool.destroy(dynamic_cast<T*>(conn));
		            break;
		        }        
		    }

		    return i;	  
		}

		template< class T >
		int ActiveReactor<T>::finish_connect(SelectionKey& key)
		{
		    SocketChannel* sc = (SocketChannel*)key.channel();
		    typename SocketConnectorMap::iterator iter = _connector_sockets.find((SelectableChannel*)sc);    

		    assert(iter != _connector_sockets.end());
		        
		    if(sc->finish_connect() )
		    {
		        sc->configure_blocking(false);
		        sc->tcp_nodelay(true);

		        key.interest_ops(SelectionKey::OP_READ);

		        ConnectionInfo* conn = iter->second._connection_info;	
		       conn->open(*(iter->second._processor));
		       conn->add_event_listener(*this);        

		        key.attach(conn);

		        return 0;
		    }
		    else
		    {
		        iter->second._connection_info->close();
				long timer_id = _timer_container->schedule(*this, _reconnect_interval, 0, (void *)sc);
		        if(timer_id < 0)
		        {
		            async_connect(iter->second);
		        }
		        return -1;
		    }

		}

		template< class T >
		void ActiveReactor<T>::handle_timeout(void *param)
		{
		    typename SocketConnectorMap::iterator iter = _connector_sockets.find((SelectableChannel*)param);
		    if(iter == _connector_sockets.end())
		    {
		        return;
		    }
		    async_connect(iter->second);
		}

		template< class T >
		void ActiveReactor<T>::connection_event(ConnectionEvent& e)
		{
		    if(ConnectionEvent::CONNECTION_CLOSED == e.get_event_code())
		    {
		        _to_unregister.push_back(&e.get_connection());
		    }
		}

		template< class T >
		int ActiveReactor<T>::register_new_channels()
		{
		    //注册新的channel
		    for (ConnectionInfoList::iterator iter = _to_register.begin(); iter != _to_register.end(); iter++)
		    {
		        ConnectionInfo* conn = *iter;
		        SocketChannel& incoming_channel  = conn->get_channel();

		        incoming_channel.configure_blocking(false);
		        incoming_channel.tcp_nodelay(true);
		   
		        SelectionKey* sk = incoming_channel.enregister(*_selector, 0, conn);

				if(NULL != sk)
				{
		        	//同时将SelectionKey标记为可读，以便读取。
		        	sk->interest_ops(SelectionKey::OP_READ);
		        	conn->open();
		        	conn->add_event_listener(*this);
				}
				else
				{
					//just close it
					conn->close();
				}
		    }
		    _to_register.clear();

		    //删除关闭的channel
		    for (ConnectionInfoList::iterator iter = _to_unregister.begin(); iter != _to_unregister.end(); iter++)
		    {
		        ConnectionInfo* ci = *iter;
		        //如果该channel在_connector_sockets中，自动重连
		        SelectableChannel* channel = &(ci->get_channel());

		        typename SocketConnectorMap::iterator iter_connector = _connector_sockets.find(channel);
		        if(iter_connector != _connector_sockets.end())
		        {
		            //reconnect
		            async_connect(iter_connector->second);
		        }
		        else
		        {
		            _connection_pool.destroy( dynamic_cast<T*>(ci));
		        }
		        
		    }

		    _to_unregister.clear();

		    return 0;
		}
	}
}

#endif

