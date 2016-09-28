#ifndef _XDRIVE_NOTICE_SERVER_H_
#define _XDRIVE_NOTICE_SERVER_H_

#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include "app/application.h"
#include "net/connectioninfo.h"
#include "net/active_reactor.h"

using namespace log4cplus;

class NoticeServer : public lce::app::Application
{
public:
	NoticeServer(int argc,char** argv);
	~NoticeServer();
	int parse_absolute_home_path();

protected:
	int impl_init();
	void run();


private:
	lce::net::ActiveReactor<lce::net::ConnectionInfo>* _active_reactor;
};

#endif 


