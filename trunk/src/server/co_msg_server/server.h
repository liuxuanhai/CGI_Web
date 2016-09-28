#ifndef _XDRIVE_SERVER_H_
#define _XDRIVE_SERVER_H_

#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include "app/application.h"
#include "net/connectioninfo.h"
#include "net/active_reactor.h"

using namespace log4cplus;

class Server : public lce::app::Application
{
public:
	Server(int argc,char** argv);
	~Server();
	int parse_absolute_home_path();

protected:
	int impl_init();
	void run();

private:
	lce::net::ActiveReactor<lce::net::ConnectionInfo>* _active_reactor;
};

#endif 


