#include "server.h"
#include "executor_thread.h"
#include "global_var.h"
#include "net/datagramstringbuffer_factory.h"

#define s_env_home_path "CO_MSG_SERVER_HOME_PATH"

Server* g_server = NULL;
lce::memory::FixedSizeAllocator*   		g_lbb_allcator            = NULL;
ClientProcessor*                		g_client_processor        = NULL;
ExecutorThreadProcessor*        		g_executor_thread_processor = NULL;
log4cplus::Logger 						coLogger = log4cplus::Logger::getInstance("Coroutine");

ServerProcessorPA*                 		g_server_processor_pa = NULL;
ServerProcessorMsg*                 	g_server_processor_msg = NULL;



Server::Server(int argc,char** argv) : Application(argc, argv)
{

}

Server::~Server()
{

}

int Server::parse_absolute_home_path()
{
    char* path = ::getenv(s_env_home_path);		//	path = "/home/dev/hoosho/server/co_msg_server/"
    if(!path)
    {
        cerr<<"get environment(CO_MSG_SERVER_HOME_PATH) : NULL"<<endl;
        return -1;
    }

    _home_path = path;
    if(_home_path.empty())
    {
        cerr<<"get environment(CO_MSG_SERVER_HOME_PATH) : empty"<<endl;
        return -1;
    }

    return 0;
}

int Server::impl_init()
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

	g_client_processor = new ClientProcessor(stConfig.get_int_param("COROUTINE_CONTAINER", "timeout"));
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

	
	//coroutineModuleBase init
	int stCoCapacity = stConfig.get_int_param("COROUTINE_CONTAINER", "capacity");
	int stCoStackSize = stConfig.get_int_param("COROUTINE_CONTAINER", "stack_size");
	CoroutineModuleBase<TimerModule>& co_module_base = Singleton<CoroutineModuleBase<TimerModule>>::Instance();
	if(co_module_base.BaseInit(stCoCapacity, stCoStackSize) < 0)
	{
		cerr<<"coroutine module base init failed!"<<endl<<flush;
		return -1;
	}

	//coroutine TimerModule
	TimerModule& co_timer = Singleton<TimerModule>::Instance();
	co_timer.ModuleInit();

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
	
    g_server_processor_pa = new ServerProcessorPA;
    assert(g_server_processor_pa);

    
	//add tcp connector to PAServer
	std::string strConnectIp = stConfig.get_string_param("PA_SERVER", "ip");
	int iConnectPort = stConfig.get_int_param("PA_SERVER", "port");
	_active_reactor->add_connector(strConnectIp.c_str(), 
									iConnectPort, 
									*g_server_processor_pa, 
									*(new lce::net::DatagramStringBufferFactory()));

	g_server_processor_msg = new ServerProcessorMsg;
    assert(g_server_processor_msg);

    
	//add tcp connector to MsgServer
	strConnectIp = stConfig.get_string_param("MSG_SERVER", "ip");
	iConnectPort = stConfig.get_int_param("MSG_SERVER", "port");
	_active_reactor->add_connector(strConnectIp.c_str(), 
									iConnectPort, 
									*g_server_processor_msg, 
									*(new lce::net::DatagramStringBufferFactory()));

    return 0;
}

void Server::run()
{
	_active_reactor->poll();
    
    Singleton<TimerModule>::Instance().ModuleRun(); 

    usleep(10);
}

int main(int argc,char* argv[])
{
    g_server = new Server(argc, argv);
	g_server->start();
    delete g_server;
    return 0;
}

