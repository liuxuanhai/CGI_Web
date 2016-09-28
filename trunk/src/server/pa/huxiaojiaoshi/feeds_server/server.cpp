#include "server.h"
#include "executor_thread.h"
#include "global_var.h"

#define s_env_home_path "HUXIAOJIAOSHI_FEEDS_SERVER_HOME_PATH"

FeedsServer* g_server = NULL;
lce::memory::FixedSizeAllocator*   g_lbb_allcator            = NULL;
ClientProcessor*                g_client_processor        = NULL;
ServerProcessorPay*             g_server_processor_pay = NULL;
ServerProcessorTranscode*             g_server_processor_transcode = NULL;
ServerProcessorToken*                  g_server_processor_token = NULL;
ExecutorThreadProcessor*        g_executor_thread_processor = NULL;
//FeedsCachePool*              g_feeds_cache_pool = NULL;
lce::app::TimerContainer*       g_timer_container         = NULL;
FsmContainer<FsmFeeds>*      g_fsm_feeds_container  = NULL;
FsmContainer<FsmFollow>*      g_fsm_follow_container = NULL;

FeedsServer::FeedsServer(int argc,char** argv) : Application(argc, argv)
{

}

FeedsServer::~FeedsServer()
{

}

int FeedsServer::parse_absolute_home_path()
{
    char* path = ::getenv(s_env_home_path);
    if(!path)
    {
        cerr<<"get environment(FEEDS_SERVER_HOME_PATH) : NULL"<<endl;
        return -1;
    }

    _home_path = path;
    if(_home_path.empty())
    {
        cerr<<"get environment(FEEDS_SERVER_HOME_PATH) : empty"<<endl;
        return -1;
    }

    return 0;
}

int FeedsServer::impl_init()
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
    assert(g_client_processor);

    g_server_processor_pay = new ServerProcessorPay;
    assert(g_server_processor_pay);

    g_server_processor_transcode = new ServerProcessorTranscode;
    assert(g_server_processor_transcode);

    g_server_processor_token = new ServerProcessorToken;
    assert(g_server_processor_token);

    g_timer_container = new lce::app::TimerContainer(stConfig.get_int_param("TIMER_CONTAINER", "capacity"));
    if(g_timer_container->init() != 0)
    {
        cerr << "TimerContainer init fail"<<endl;
        return -1;
    }
    assert(g_timer_container);

    /*
    g_feeds_cache_pool = new FeedsCachePool(stConfig.get_int_param("USER_INFO_CACHE_POOL", "capacity"));
    if(g_feeds_cache_pool->init() < 0)
    {
    	cerr<<"user info Cache Pool init failed !"<<endl;
    	return -1;
    }
    assert(g_feeds_cache_pool);
    */

    g_fsm_feeds_container = new FsmContainer<FsmFeeds>(stConfig.get_int_param("FSM_CONTAINER", "capacity"));
    if(g_fsm_feeds_container->init() < 0)
    {
        cerr<<"fsm feeds container init failed !"<<endl;
        return -1;
    }
    assert(g_fsm_feeds_container);

    g_fsm_follow_container = new FsmContainer<FsmFollow>(stConfig.get_int_param("FSM_CONTAINER", "capacity"));
    if(g_fsm_follow_container->init() < 0)
    {
        cerr<<"fsm follow container init failed !"<<endl;
        return -1;
    }
    assert(g_fsm_follow_container);



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

    lce::net::DatagramStringBufferFactory* pDatagramFactory = new  lce::net::DatagramStringBufferFactory();
    //add tcp acceptor to reactor
    string strListenIp = stConfig.get_string_param("LISTEN", "ip");
    uint16_t wListenPort = stConfig.get_int_param("LISTEN", "port");
    if(_active_reactor->add_acceptor(("*"==strListenIp)?NULL:strListenIp.c_str()
                                     , wListenPort
                                     , *g_client_processor
                                     , *pDatagramFactory)<0)
    {
        cerr<<"add tcp acceptor failed!! ip: "<<strListenIp
            <<", port: "<<wListenPort<<endl<<flush;
        return -1;
    }

    //add tcp connector to PayServer
    strListenIp = stConfig.get_string_param("PAY_SERVER", "ip");
    wListenPort = stConfig.get_int_param("PAY_SERVER", "port");
    _active_reactor->add_connector(strListenIp.c_str(), wListenPort, *g_server_processor_pay, *pDatagramFactory);

    //add tcp connector to TranscodeServer
    strListenIp = stConfig.get_string_param("TRANSCODE_SERVER", "ip");
    wListenPort = stConfig.get_int_param("TRANSCODE_SERVER", "port");
    _active_reactor->add_connector(strListenIp.c_str(), wListenPort, *g_server_processor_transcode, *pDatagramFactory);

    //add tcp connector to TokenServer
    strListenIp = stConfig.get_string_param("TOKEN_SERVER", "ip");
    wListenPort = stConfig.get_int_param("TOKEN_SERVER", "port");
    _active_reactor->add_connector(strListenIp.c_str(), wListenPort, *g_server_processor_token, *pDatagramFactory);

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

void FeedsServer::run()
{
    g_executor_thread_processor->poll();

    _active_reactor->poll();

    g_timer_container->poll();
}

int main(int argc,char* argv[])
{
    g_server = new FeedsServer(argc, argv);
    g_server->start();
    delete g_server;
    return 0;
}

