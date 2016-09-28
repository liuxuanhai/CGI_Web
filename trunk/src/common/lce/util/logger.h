#ifndef _LCE_UTIL_LOGGER_H_
#define _LCE_UTIL_LOGGER_H_

#ifdef LOGGER

#include "log4cplus/logger.h"
#include "log4cplus/configurator.h"
#include "log4cplus/loggingmacros.h"
using namespace log4cplus;

#define DECL_LOGGER(logger) static log4cplus::Logger logger;
#define IMPL_LOGGER(classname, logger) log4cplus::Logger classname::logger = log4cplus::Logger::getInstance(#classname);
#define TEMPLATE_IMPL_LOGGER(classname, logger) template<class T> log4cplus::Logger classname<T>::logger = log4cplus::Logger::getInstance(#classname);
#define TEMPLATE_IMPL_LOGGER_2(classname, logger) template<class T, class T2> log4cplus::Logger classname<T, T2>::logger = log4cplus::Logger::getInstance(#classname);
#define TEMPLATE_IMPL_LOGGER_3(classname, logger) template<class T, class T2, class T3> log4cplus::Logger classname<T, T2, T3>::logger = log4cplus::Logger::getInstance(#classname);

#else
#define LOG4CPLUS_TRACE(a,b)
#define LOG4CPLUS_DEBUG(a,b)
#define LOG4CPLUS_INFO(a,b)
#define LOG4CPLUS_WARN(a,b)
#define LOG4CPLUS_ERROR(a,b)
#define LOG4CPLUS_FATAL(a,b)

#define DECL_LOGGER(logger)
#define IMPL_LOGGER(classname, logger)
#define TEMPLATE_IMPL_LOGGER(classname, logger)
#define TEMPLATE_IMPL_LOGGER_2(classname, logger)
#define TEMPLATE_IMPL_LOGGER_3(classname, logger)

#endif

#endif