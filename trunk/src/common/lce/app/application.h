#ifndef _APPLICATION_H_
#define _APPLICATION_H_
#include <sys/shm.h>
#include <string>
#include <cstring>
#include <cstdlib>
#include "config.h"

using namespace std;

namespace lce
{
    namespace app
    {
        class Application
        {
        protected:
            Application()
            {
            }
            Application(int argc, char* argv[]);
            virtual ~Application();

        public:
            static void sig_handler(int signal);
            static int  send_signal(pid_t pid, int sig);  
            static void init_daemon();
            
            const string& home_path(){return _home_path;}
            const Config& config(){return _config;}
            bool get_stopped(){return _stopped;}
            bool set_stopped(bool stopped){return _stopped = stopped;}
            void  set_version(const string& version){_version = version;}
            const string& get_version(){return _version;}
            const string& get_programname(){return _programname;}
            
        public:
	        int init();
            void start();
            void stop();
            
		protected:
            int parse_params();
            
			int signal_server();
			void usage();

			virtual int parse_absolute_home_path(){return -1;}
            virtual int impl_init(){return 0;}
            virtual void run() = 0;

        protected:
            string        _home_path;
            string        _programname;
            string        _dash_k_arg;
            
            int           _argc;
            char**        _argv;
            bool          _daemon;
            string        _version;

			static Config        _config;
            static bool 		  _stopped;
        };
    }
}


#endif
