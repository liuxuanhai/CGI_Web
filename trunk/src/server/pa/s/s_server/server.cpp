#include "server.h"
#include "executor_thread_user.h"
#include "executor_thread_feed.h"
#include "global_var.h"
#include "net/datagramstringbuffer_factory.h"

#define s_env_home_path "S_SERVER_HOME_PATH"

ZServer* g_server = NULL;
lce::memory::FixedSizeAllocator*   g_lbb_allcator            = NULL;
ClientProcessor*                g_client_processor        = NULL;
TokenServerProcessor*              g_token_server_processor = NULL;
ExecutorThreadProcessor*        g_executor_thread_processor = NULL;
lce::app::TimerContainer*       g_timer_container         = NULL;
FsmContainer<FsmBusiness>*      g_fsm_business_container = NULL;

ZServer::ZServer(int argc,char** argv) : Application(argc, argv)
{

}

ZServer::~ZServer()
{

}

int ZServer::parse_absolute_home_path()
{
    char* path = ::getenv(s_env_home_path);
    if(!path)
    {
        cerr<<"get environment(Z_SERVER_HOME_PATH) : NULL"<<endl;
        return -1;
    }

    _home_path = path;
    if(_home_path.empty())
    {
        cerr<<"get environment(Z_SERVER_HOME_PATH) : empty"<<endl;
        return -1;
    }

    return 0;
}

int ZServer::impl_init()
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

	//timer
	g_timer_container = new lce::app::TimerContainer(stConfig.get_int_param("TIMER_CONTAINER", "capacity"));
    if(g_timer_container->init() != 0)
    {
        cerr << "TimerContainer init fail"<<endl;
        return -1;
    }
    assert(g_timer_container);

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
        ExecutorThreadUser* threadUser = new ExecutorThreadUser();
        if(threadUser->init(g_executor_thread_processor->get_user_queue(i)) <0)
        {
            return -1;
        }
        threadUser->start();        

        ExecutorThreadFeed* threadFeed = new ExecutorThreadFeed();
        if(threadFeed->init(g_executor_thread_processor->get_feed_queue(i)) <0)
        {
            return -1;
        }
        threadFeed->start();        
    }

	//fsm business
	g_fsm_business_container = new FsmContainer<FsmBusiness>(stConfig.get_int_param("FSM_CONTAINER", "capacity"));
	if(g_fsm_business_container->init() < 0)
	{
		cerr<<"business fsm container init failed !"<<endl;
		return -1;
	}
    assert(g_fsm_business_container);
	
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
    g_token_server_processor = new TokenServerProcessor();
	assert(g_client_processor && g_token_server_processor);

	if(g_client_processor->init() < 0)
	{
		cerr << "ClientProcessor init fail" << endl << flush;
		return -1;
	}

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
	
    //add tcp acceptor to client
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

    //add tcp connector to token server
	string strTokenServerIp = stConfig.get_string_param("TOKEN_SERVER", "ip"); 
	uint16_t wTokenServerPort  = stConfig.get_int_param("TOKEN_SERVER", "port"); 
    _active_reactor->add_connector(strTokenServerIp.c_str(), wTokenServerPort, *g_token_server_processor, *pCommonDatagramFactory);    
    
    return 0;
}

void ZServer::run()
{
	g_executor_thread_processor->poll();

	_active_reactor->poll();
    
    g_timer_container->poll();
}

int main(int argc,char* argv[])
{
    g_server = new ZServer(argc, argv);
	g_server->start();
    delete g_server;
    return 0;
}

