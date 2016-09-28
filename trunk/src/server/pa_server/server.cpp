#include "server.h"
#include "executor_thread.h"
#include "global_var.h"
#include "net/datagramstringbuffer_factory.h"

#define s_env_home_path "PA_SERVER_HOME_PATH"

const char* HX_PA_APPID = NULL; //"wxfd93b50f7122f71b"
const char* HX_PA_APPSECRET = NULL; // "26a8bb809b08e62b45be236e9ea0f36d"
std::string debugPAAppId = "";
std::string debugPAAppSecret = "";
PAServer* g_server = NULL;
lce::memory::FixedSizeAllocator*   g_lbb_allcator            = NULL;
ClientProcessor*                g_client_processor        = NULL;
ExecutorThreadProcessor*        g_executor_thread_processor = NULL;
lce::app::TimerContainer*       g_timer_container         = NULL;
FsmContainer<FsmPreAuth>*       g_fsm_preauth_container = NULL;
FsmContainer<FsmResource>*      g_fsm_resource_container = NULL;
FsmContainer<FsmBusiness>*      g_fsm_business_container = NULL;


PAServer::PAServer(int argc,char** argv) : Application(argc, argv)
{

}

PAServer::~PAServer()
{

}

int PAServer::parse_absolute_home_path()
{
    char* path = ::getenv(s_env_home_path);
    if(!path)
    {
        cerr<<"get environment(PA_SERVER_HOME_PATH) : NULL"<<endl;
        return -1;
    }

    _home_path = path;
    if(_home_path.empty())
    {
        cerr<<"get environment(PA_SERVER_HOME_PATH) : empty"<<endl;
        return -1;
    }

    return 0;
}

int PAServer::impl_init()
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

	//debug PA appid appsecret info
	//debug=0
	//appid=xxxx
	//appsecret=yyyy
	if(stConfig.get_int_param("PA_TOKEN", "debug"))
	{
		debugPAAppId = stConfig.get_string_param("PA_TOKEN", "appid");
		debugPAAppSecret = stConfig.get_string_param("PA_TOKEN", "appsecret");
		HX_PA_APPID = debugPAAppId.c_str();
		HX_PA_APPSECRET = debugPAAppSecret.c_str();
	}
	else
	{
		HX_PA_APPID = "wxfd93b50f7122f71b";
		HX_PA_APPSECRET = "26a8bb809b08e62b45be236e9ea0f36d";
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
        ExecutorThread* thread = new ExecutorThread();
        if(thread->init(g_executor_thread_processor->get_queue(i)) <0)
        {
            return -1;
        }
		
        thread->start();
    }

	//fsm preauth
	g_fsm_preauth_container = new FsmContainer<FsmPreAuth>(stConfig.get_int_param("FSM_CONTAINER", "capacity"));
	if(g_fsm_preauth_container->init() < 0)
	{
		cerr<<"pre auth fsm container init failed !"<<endl;
		return -1;
	}
    assert(g_fsm_preauth_container);

	//fsm resource
	g_fsm_resource_container = new FsmContainer<FsmResource>(stConfig.get_int_param("FSM_CONTAINER", "capacity"));
	if(g_fsm_resource_container->init() < 0)
	{
		cerr<<"resource fsm container init failed !"<<endl;
		return -1;
	}
    assert(g_fsm_resource_container);

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
	assert(g_client_processor);

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

    //add tcp acceptor to reactor
    string strListenIp = stConfig.get_string_param("LISTEN", "ip");
    uint16_t wListenPort = stConfig.get_int_param("LISTEN", "port");
    if(_active_reactor->add_acceptor(("*"==strListenIp)?NULL:strListenIp.c_str()
									   , wListenPort
									   , *g_client_processor
                                       , *(new lce::net::DatagramStringBufferFactory()))<0)
    {
        cerr<<"add tcp acceptor failed!! ip: "<<strListenIp
                        <<", port: "<<wListenPort<<endl<<flush;
        return -1;
    }

    return 0;
}

void PAServer::run()
{
	g_executor_thread_processor->poll();

	_active_reactor->poll();
    
    g_timer_container->poll();
}

int main(int argc,char* argv[])
{
    g_server = new PAServer(argc, argv);
	g_server->start();
    delete g_server;
    return 0;
}

