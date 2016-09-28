#ifndef _COMM_DEF_H_
#define _COMM_DEF_H_

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>
#include <assert.h>
#include <getopt.h>
#include <libgen.h>
#include <malloc.h>
#include <sys/times.h>

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <ucontext.h>

#include <iostream>
#include <string>
#include <sstream>
#include <list>
#include <vector>
#include <map>
#include <queue>

#include "singleton.h"

#define PKG_BUF_SIZE 10240*10

// 表示该参数只是输入参数
#ifndef IN
#define IN
#endif


// 表示该参数只是输出参数
#ifndef OUT
#define OUT
#endif


// 表示该参数既是输入参数，又是输出参数
#ifndef INOUT
#define INOUT
#endif


#define LOG_FATAL(fmt, args...) \
	char buffer[1000]; \
	sprintf(buffer, fmt, ##args); \
	LOG4CPLUS_FATAL(coLogger, buffer);

#define LOG_ERROR(fmt, args...) \
	char buffer[1000]; \
	sprintf(buffer, fmt, ##args); \
	LOG4CPLUS_ERROR(coLogger, buffer);

#define LOG_INFO(fmt, args...) \
	char buffer[1000]; \
	sprintf(buffer, fmt, ##args); \
	LOG4CPLUS_INFO(coLogger, buffer);

#define LOG_DEBUG(fmt, args...) \
	char buffer[1000]; \
	sprintf(buffer, fmt, ##args); \
	LOG4CPLUS_DEBUG(coLogger, buffer);
	

#define ASSERT(condition, fmt, args...) \
    do { \
        if (!(condition)) { \
            LOG_FATAL(fmt, ##args); \
            assert(NULL == #condition " failed in " __FILE__); \
        } \
    } while(0)


#define DISALLOW_COPY_AND_ASSIGN(TypeName)          \
    TypeName(const TypeName&) = delete;             \
    TypeName& operator=(const TypeName&) = delete

#define ROUNDUP(n, width) (((n) + (width) - 1) & ~((width) - 1))

#define ROUNDDOWN(n, width) ((n) & ~(-width))

// UP(A/B) = int((A+B-1)/B)
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

#define UNUSE_ARG(arg) ((void)arg)

#define MARCO_ARGS_CONS(a, b) a##b

#define SYNTHESIZE(VarType, VarName) \
    private: VarType MARCO_ARGS_CONS(VarName, _); \
    public: inline VarType VarName(void) const { return MARCO_ARGS_CONS(VarName, _); } \
    public: inline void set_##VarName(VarType VarName) { MARCO_ARGS_CONS(VarName, _) = VarName; }

#define SYNTHESIZE_REF(VarType, VarName) \
    private: VarType MARCO_ARGS_CONS(VarName, _); \
    public: inline VarType* mutable_##VarName(void) { return &(MARCO_ARGS_CONS(VarName, _)); }\
    public: inline VarType& VarName(void) { return MARCO_ARGS_CONS(VarName, _); } \
    public: inline const VarType& const_##VarName(void) const {return  MARCO_ARGS_CONS(VarName, _);} \
    public: inline void set_##VarName(const VarType& VarName) { MARCO_ARGS_CONS(VarName, _) = VarName; }

#define SYNTHESIZE_CHAR_ARRAY(VarName, VarLength) \
    private: char MARCO_ARGS_CONS(VarName, _)[VarLength]; \
    public: inline const char* VarName(void) const { return MARCO_ARGS_CONS(VarName, _); } \
    public: inline void set_##VarName(const char* VarName) { strncpy(MARCO_ARGS_CONS(VarName, _), VarName, VarLength);}

#define SYNTHESIZE_ARRAY(VarType, VarName, VarLength) \
    private: VarType MARCO_ARGS_CONS(VarName, _)[VarLength]; \
    public: inline VarType* mutable_##VarName(int32_t index) \
        { \
            if (index < 0 || index >= VarLength) \
                return NULL; \
            return &(MARCO_ARGS_CONS(VarName, _)[index]); \
        } \
    public: inline const VarType& VarName(int32_t index) \
        { \
            return MARCO_ARGS_CONS(VarName, _)[index]; \
        } \
    public: inline void set_##VarName(int32_t index, const VarType& VarName) \
        { \
            MARCO_ARGS_CONS(VarName, _)[index] = VarName; \
        } \
    public: inline void clear_##VarName() \
        { \
            memset(&MARCO_ARGS_CONS(VarName, _), 0, sizeof(MARCO_ARGS_CONS(VarName, _))); \
        } \
    public: inline int32_t VarName##_size() { return VarLength; }


#define CHECK_FUNC_RET(func) do { int ret = func(); if (ret != 0) return ret;} while(0)

#define SAFE_DELETE(p) do { delete (p); (p) = NULL; } while(0)
#define SAFE_DELETE_ARRAY(p) do { delete [] (p); (p) = NULL; } while(0)
#define SAFE_FREE(p) do { free (p); (p) = NULL; } while(0)

#define LOG_DEBUG_FUNC_NAME LOG_DEBUG("%s", __PRETTY_FUNCTION__)

#define MAX(a, b)	((a) > (b) ? (a) : (b))
#define MIN(a, b)	((a) < (b) ? (a) : (b))

#endif
