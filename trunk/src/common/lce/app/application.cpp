#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include "log4cplus/logger.h"
#include "log4cplus/configurator.h"


#include "pid.h"
#include "application.h"

#define DEFAULT_CONFIG_PATH "conf/server.conf"
#define DEFAULT_LOGGER_CONFIG_PATH "conf/logger.properties"


namespace lce
{
    namespace app
    {
    	bool Application::_stopped = false;
    	Config Application:: _config;
    	
    	//进程默认的信号处理器， 停止主循环
		void Application::sig_handler(int signal)
        {
            cout<<"received signal"<<signal<<endl;
            _stopped = true;
        }

        int Application::send_signal(pid_t pid, int sig)
        {
            if (::kill(pid, sig) < 0) 
            {
                printf("sending signal to server failed, error =[%s]", strerror(errno));
                return -1;
            }
            return 0;
        }

		void Application::init_daemon()
        {
            pid_t pid;

            pid = fork();
            if (pid > 0)
            {
				exit(0);
            }   
            else if (pid < 0)
            {
                perror("fail to fork.");
                exit(1);
            }

            /* now the process is the leader of the session, the leader of its process grp, and have no controlling tty */
            if(setsid() == - 1)
            {
                perror("omail: setsid() fail. It seems impossible.");
                exit(1);
            }
            setpgid(0, 0);

            /*** change umask */
            umask(0027);

            /* ignore the SIGHUP, in case the session leader sending it when terminating.  */
            signal(SIGHUP, SIG_IGN);
            signal(SIGINT, SIG_IGN);
            signal(SIGQUIT, SIG_IGN);
            signal(SIGPIPE, SIG_IGN);
            signal(SIGURG, SIG_IGN);
            signal(SIGTSTP, SIG_IGN);
            signal(SIGTTIN, SIG_IGN);
            signal(SIGTTOU, SIG_IGN);
            signal(SIGALRM, SIG_IGN);
            signal(SIGCHLD, SIG_IGN);
            //signal(SIGTERM, SIG_IGN);

            pid = fork();
            if (pid > 0)
                exit(0);
            else if (pid < 0)
            {
                perror("fail to fork.");
                exit(1);
            }

            //先只吧标准输入关闭吧， 业务有需求，自行选择fd关闭好了
            close(0);
        }
        

        Application::Application(int argc, char** argv)
                :_argc(argc)
                , _argv(argv)
                , _daemon(false)

        {
            char* ptr = strrchr(argv[0], '/');
            if (ptr == NULL) 
            {
                _programname =argv[0];
            } 
            else 
            {
                _programname = (ptr + 1);
            }
        }

        Application::~Application()
        {

        }

        void Application::usage()
        {
            cout<<get_programname()<<" [-k [start|stop|restart]|-d|-v|-h]"<<endl;
        }
            
        int Application::signal_server()
        {
            int rv;
            pid_t otherpid;
            bool running = false;
            char status[256];
            
            string pidfile = _home_path+"/pid/"+_programname+".pid";

            rv = Pid::read_pid(pidfile.c_str(), otherpid);
            if (rv < 0) 
            {
                snprintf(status, 256,"%s not running !\n", _programname.c_str());
            }
            else 
            {
                if (Pid::exists_pid(otherpid)) 
                {
                    running = true;
                    snprintf(status, 256,"%s (pid %d) already running !\n", _programname.c_str(), otherpid);
                }
                else 
                {
                    snprintf(status, 256,"%s (pid %d) not running ! \n", _programname.c_str(), otherpid);
                }
            }
            
            if (!strcmp(_dash_k_arg.c_str(), "start")) 
            {
                if (running) 
                {
                    printf("%s\n", status);
                    return -1;
                }
            }

            if (!strcmp(_dash_k_arg.c_str(), "stop")) 
            {
                if (!running)
                {
                    printf("%s\n", status);
                }
                else
                {
                    send_signal(otherpid, SIGTERM);
                }
                return -1;
            }

            if (!strcmp(_dash_k_arg.c_str(), "restart")) 
            {
                if (!running) 
                {
                    printf("%s not running, trying to start\n",  _programname.c_str());
                }
                else 
                {
                    send_signal(otherpid, SIGHUP);
                    return -1;
                }
            }

            if (!strcmp(_dash_k_arg.c_str(), "graceful")) 
            {
                if (!running) 
                {
                    printf("%s not running, trying to start\n",  _programname.c_str());
                }
                else 
                {
                    send_signal(otherpid, SIGUSR1);
                    return -1;
                }
            }

            return 0;
        }

        int Application::parse_params()
        {
            char option;
            while((option = getopt(_argc, _argv, "k:dvh"))!=-1)
            {
                switch(option)
                {
                case 'k':
                    _dash_k_arg = ::optarg;
                    break;
                    
                case 'd':
                    _daemon = true;
                    break;
                    
                case 'v':
                    cout<<_programname<<" version is:"<<_version<<endl;
                    exit(0);
                    
                case 'h':
                    usage();
                    exit(0);
                    
                default:
                    cout<<"unknown option ["<<option<<"]"<<endl;
                    return -1;
                }

            }

            return 0;

        }
        
        int Application::init()
        {
            signal(SIGTERM, sig_handler);
            signal(SIGPIPE, SIG_IGN);

			//设定server部署的绝对路径
			int rv = parse_absolute_home_path();
            if(rv < 0)
            {
                cerr<<"parse home path failed\n";
                return -1;
            }

			//解析命令行参数
           	rv = parse_params();
            if(rv < 0)
            {
                usage();
                return -1;
            }
               
            if(!_dash_k_arg.empty())
            {
                if(signal_server() < 0)
                {
                    return -1;
                }
            }

            //daemon方式运行进程
            if(_daemon)
            {
                init_daemon();
            }

			//初始化配置
			string server_conf_path = _home_path + "/" + DEFAULT_CONFIG_PATH;
			if(_config.load(server_conf_path) < 0)
			{
				cerr<<"config load failed, server_conf_path="<<server_conf_path<<"\n";
                return -1;
			}

			//初始化日志
			string server_logger_conf_path = _home_path + "/" + DEFAULT_LOGGER_CONFIG_PATH;
			log4cplus::PropertyConfigurator::doConfigure(server_logger_conf_path);

			//业务自己的资源初始化
            rv = impl_init();
            if(rv >= 0)
            {
                string pid_file = _home_path + "/pid/"+ _programname + ".pid";
                Pid::write_pid(pid_file.c_str());
            }

            return rv;
        }

        void Application::start()
        {
            int rv = init();
            if(rv < 0)
            {
                set_stopped(true);
            }
            
            while(!get_stopped())
            {
                run();
            }

            stop();
        }

        void Application::stop()
        {
            cout<<_programname<<" stopped"<<endl;
        }
    }
}
