#include "server.h"
#include "executor_thread.h"
#include "global_var.h"
#include "net/datagramstringbuffer_factory.h"

#define s_env_home_path "MSG_SERVER_HOME_PATH"

MsgServer* g_server = NULL;
lce::memory::FixedSizeAllocator*   	g_lbb_allcator            = NULL;
ClientProcessor*                	g_client_processor        = NULL;
ServerProcessorNotice*			  	g_server_processor_notice = NULL;
ExecutorThreadProcessor*        	g_executor_thread_processor = NULL;
MsgContentCachePool*              	g_msg_content_cache_pool = NULL;
lce::app::TimerContainer*       	g_timer_container         = NULL;
FsmContainer<MsgFsm>*      			g_msg_content_fsm_container  = NULL;

MsgServer::MsgServer(int argc,char** argv) : Application(argc, argv)
{

}

MsgServer::~MsgServer()
{

}

int MsgServer::parse_absolute_home_path()
{
    char* path = ::getenv(s_env_home_path);		//	path = "/home/dev/hoosho/server/msg_server/"
    if(!path)
    {
        cerr<<"get environment(MSG_SERVER_HOME_PATH) : NULL"<<endl;
        return -1;
    }

    _home_path = path;
    if(_home_path.empty())
    {
        cerr<<"get environment(MSG_SERVER_HOME_PATH) : empty"<<endl;
        return -1;
    }

    return 0;
}

int MsgServer::impl_init()
{
	const lce::app::Config& stConfig = config();

	//sys resource limit 
    struct rlimit limit;
    limit.rlim_cur = stConfig.get_int_param("LISTEN", "max_connections")*2;
    limit.rlim_max = limit.rlim_cur ;
    if(setrlimit(RLIMIT_NOFILE,&limit) < 0)
    {
        cerr<<"setrlimit failed." << endl;
        return -1;
    }

	//global vars init
    g_lbb_allcator = new lce::memory::FixedSizeAllocator(
    		  stConfig.get_int_param("FIX_SIZE_ALLOCATOR", "page.size.KB")*1024
            , stConfig.get_int_param("FIX_SIZE_ALLOCATOR", "page.size.KB")*1024
            , stConfig.get_int_param("FIX_SIZE_ALLOCATOR", "capacity.size.MB")*1024*1024);
    assert(g_lbb_allcator);
    if(g_lbb_allcator->init()<0)
    {
        cerr<<"new FixedSizeAllocator failed"<<endl;
        return -1;
    }

	g_client_processor = new ClientProcessor;
	g_server_processor_notice = new ServerProcessorNotice;
	assert(g_client_processor && g_server_processor_notice);
	
    g_timer_container = new lce::app::TimerContainer(stConfig.get_int_param("TIMER_CONTAINER", "capacity"));
    if(g_timer_container->init() != 0)
    {
        cerr << "TimerContainer init fail"<<endl;
        return -1;
    }
    assert(g_timer_container);
    
	g_msg_content_cache_pool = new MsgContentCachePool(stConfig.get_int_param("USER_INFO_CACHE_POOL", "capacity"));
	if(g_msg_content_cache_pool->init() < 0)
	{
		cerr<<"user info Cache Pool init failed !"<<endl;
		return -1;
	}
	assert(g_msg_content_cache_pool);
	
	g_msg_content_fsm_container = new FsmContainer<MsgFsm>(stConfig.get_int_param("FSM_CONTAINER", "capacity"));
	if(g_msg_content_fsm_container->init() < 0)
	{
		cerr<<"user fsm container init failed !"<<endl;
		return -1;
	}
    assert(g_msg_content_fsm_container);

    //reactor
    _active_reactor = new lce::net::ActiveReactor<lce::net::ConnectionInfo>(g_lbb_allcator
            , lce::net::ActiveReactor<lce::net::ConnectionInfo>::DEFAULT_SELECT_TIMEOUT
            , 5000
            , lce::net::ActiveReactor<lce::net::ConnectionInfo>::DEFAULT_ACCECPT_ONCE
            , lce::net::ActiveReactor<lce::net::ConnectionInfo>::DEFAULT_BACKLOG
                                                            );
    if(_active_reactor->init() < 0)
    {
        cerr <<"active_reactor init fail" <<endl <<flush;
        return -1;
    }

    lce::net::DatagramStringBufferFactory* pCommonDatagramFactory = new lce::net::DatagramStringBufferFactory();

    //add tcp acceptor to reactor
    string strListenIp = stConfig.get_string_param("LISTEN", "ip");
    uint16_t wListenPort = stConfig.get_int_param("LISTEN", "port");
    if(_active_reactor->add_acceptor(("*"==strListenIp)?NULL:strListenIp.c_str()
									   , wListenPort
									   , *g_client_processor
                                       , *pCommonDatagramFactory)<0)
    {
        cerr<<"add tcp acceptor failed!! ip: "<<strListenIp
                        <<", port: "<<wListenPort<<endl<<flush;
        return -1;
    }

    //add tcp connector to NoticeServer
    /*
    std::string strConnectIp = stConfig.get_string_param("NOTICE_SERVER", "ip");
    int iConnectPort = stConfig.get_int_param("NOTICE_SERVER", "port");
	_active_reactor->add_connector(strConnectIp.c_str(), iConnectPort, *g_server_processor_notice, *pCommonDatagramFactory);
	*/


    //thread queue
	g_executor_thread_processor = new ExecutorThreadProcessor();
    if(g_executor_thread_processor->init(stConfig.get_int_param("THREAD_QUEUE", "size")
    			, stConfig.get_int_param("THREAD_QUEUE", "capacity")) != 0)
    {
        cerr << "ExecutorThreadProcessor init fail"<<endl;
        return -1;
    }
    for(int i=0; i<stConfig.get_int_param("THREAD_QUEUE", "size"); i++)
    {
        ExecutorThread* thread = new ExecutorThread();
        if(thread->init(g_executor_thread_processor->get_queue(i)) <0)
        {
            return -1;
        }
		
        thread->start();
    }

    return 0;
}

void MsgServer::run()
{
	g_executor_thread_processor->poll();

	_active_reactor->poll();
    
    g_timer_container->poll();
}

int main(int argc,char* argv[])
{
    g_server = new MsgServer(argc, argv);
	g_server->start();
    delete g_server;
    return 0;
}

