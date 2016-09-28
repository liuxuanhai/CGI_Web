#include "server.h"
#include "executor_thread.h"
#include "global_var.h"
#include "cgi/cgi_mysql.h"
#include "net/datagramstringbuffer_factory.h"

#define s_env_home_path "CONFIG_SERVER_HOME_PATH"

ConfigServer* g_server = NULL;
lce::memory::FixedSizeAllocator*   g_lbb_allcator            = NULL;
ClientProcessor*                g_client_processor        = NULL;
ExecutorThreadProcessor*        g_executor_thread_processor = NULL;
lce::app::TimerContainer*       g_timer_container         = NULL;
uint64_t						g_hoosho_no_account       = 0;


ConfigServer::ConfigServer(int argc,char** argv) : Application(argc, argv)
{

}

ConfigServer::~ConfigServer()
{

}

int ConfigServer::parse_absolute_home_path()
{
    char* path = ::getenv(s_env_home_path);
    if(!path)
    {
        cerr<<"get environment(CONFIG_SERVER_HOME_PATH) : NULL"<<endl;
        return -1;
    }

    _home_path = path;
    if(_home_path.empty())
    {
        cerr<<"get environment(CONFIG_SERVER_HOME_PATH) : empty"<<endl;
        return -1;
    }

    return 0;
}

int ConfigServer::impl_init()
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

    //thread queue,  only one queue
	g_executor_thread_processor = new ExecutorThreadProcessor();
    if(g_executor_thread_processor->init(1, 5000) != 0)
    {
        cerr << "ExecutorThreadProcessor init fail"<<endl;
        return -1;
    }
    ExecutorThread* thread = new ExecutorThread();
    if(thread->init(g_executor_thread_processor->get_queue(0)) <0)
    {
        return -1;
    }	
    thread->start();

	//lbb	
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

    //int hoosho_no_account
    {
		lce::cgi::CMysql mysql;
    	assert(mysql.Init(stConfig.get_string_param("DB", "ip")
                , stConfig.get_string_param("DB", "db_name")
                , stConfig.get_string_param("DB", "user")
                , stConfig.get_string_param("DB", "passwd")));
        std::ostringstream oss;
        oss.str("");
        oss<<"select * from "<<stConfig.get_string_param("DB", "table_name") 
        	<<" where c_key=1";
        if(!mysql.Query(oss.str()))
	    {
	        cerr<<"mysql query error, sql="<<oss.str()<<", msg="<<mysql.GetErrMsg()<<endl<<flush;
	        return -1;
	    }

	    if(mysql.GetRowCount() == 0 || !mysql.Next())
	    {
			cerr<<"mysql query error, sql="<<oss.str()<<", record not exists!!!"<<endl<<flush;
			return -1;
	    }

	    g_hoosho_no_account = strtoul(mysql.GetRow(1), NULL, 10);
	}

    return 0;
}

void ConfigServer::run()
{
	g_executor_thread_processor->poll();

	_active_reactor->poll();
    
    g_timer_container->poll();
}

int main(int argc,char* argv[])
{
    g_server = new ConfigServer(argc, argv);
	g_server->start();
    delete g_server;
    return 0;
}

