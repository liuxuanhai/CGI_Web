#ifndef _XDRIVE_FEEDS_SERVER_H_
#define _XDRIVE_FEEDS_SERVER_H_

#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include "app/application.h"
#include "net/connectioninfo.h"
#include "net/active_reactor.h"

using namespace log4cplus;

class FeedsServer : public lce::app::Application
{
public:
    FeedsServer(int argc,char** argv);
    ~FeedsServer();
    int parse_absolute_home_path();

protected:
    int impl_init();
    void run();


private:
    lce::net::ActiveReactor<lce::net::ConnectionInfo>* _active_reactor;
};

#endif


