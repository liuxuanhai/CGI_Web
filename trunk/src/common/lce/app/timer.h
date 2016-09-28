#ifndef _TIMER_H_
#define _TIMER_H_

/********************
  Timer useed in linux kernel
*********************/

#include <stdint.h>
#include <sys/time.h>
#include <list>
#include <ext/hash_map>
#include "memory/array_object_pool.h"
#include "util/list.h"

using namespace std;
using namespace __gnu_cxx;
using namespace lce::memory;

namespace lce
{
    namespace app
    {   
        class TimerHandler
        {
        public:
            virtual ~TimerHandler(){}

        public:
            virtual void handle_timeout(void* param)=0;
        };

        class TimerContainer
        {
        private:
            enum
            {
                TVN_BITS = 6,
                TVR_BITS  = 8,
                TVN_SIZE  = (1 << TVN_BITS),
                TVR_SIZE  = (1 << TVR_BITS),
                TVN_MASK  = (TVN_SIZE - 1),
                TVR_MASK = (TVR_SIZE - 1),
            };

            class Timer
            {
            public:
                Timer()
                        :_id(0),
                        _handler(NULL),
                        _expires(0),
                        _interval(0),
                        _canceled(false),                
                        _param(NULL)
                {}

                Timer (unsigned long timer_id,
                       const TimerHandler& handler,
                       void* param,
                       const time_t& expires,
                       const time_t& interval)
                        :_id( timer_id),
                        _handler((TimerHandler*)&handler),
                        _expires(expires),
                        _interval(interval),
                        _canceled(false),
                        _param(param)
                {
                }

                ~Timer(void){}

                void set (unsigned long timer_id,
                              const TimerHandler& handler,
                              void* param,
                              const time_t& expires,
                              const time_t& interval)
                {
                    _id         = timer_id;
                    _handler    = (TimerHandler*)&handler;
                    _expires    = expires;
                    _interval   = interval;
                    _param      =param;
                }

                void timeout()
                {
                    if(_handler)
                    {
                        _handler->handle_timeout(_param);
                    }
                }

                TimerHandler& get_handler (){return *_handler;}
                void set_handler (TimerHandler &handler){_handler= &handler;}

                const time_t &get_expires() const{return _expires;}
                void set_expires (const time_t &timer_value){_expires = timer_value;}

                const time_t &get_interval (void) const{return _interval;}
                void set_interval (const time_t &interval){_interval = interval;}

                bool is_canceled(){return _canceled;}
                bool cancel(){return _canceled = true;}

                unsigned long get_timer_id (void) const{return _id;}
                void set_timer_id (unsigned long timer_id){_id = timer_id;}

            public:
                unsigned long   _id;
                TimerHandler*  _handler;
                time_t _expires;
                time_t _interval;
                list_head entry;
                bool   _canceled;
                void * _param;
            };

        private:
            struct tvec_t
            {
                struct list_head vec[TVN_SIZE];
            };

            struct tvec_root_t
            {
                struct list_head vec[TVR_SIZE];
            } ;

            struct tvec_base_t
            {
                time_t  timer_jiffies;
                tvec_root_t tv1;
                tvec_t tv2;
                tvec_t tv3;
                tvec_t tv4;
                tvec_t tv5;
            } ;

            typedef hash_map<long, Timer*>  IDTimerMap;
            typedef ArrayObjectPool<Timer>    TimerPool;

        public:
            explicit TimerContainer(size_t capacity);
            TimerContainer(time_t base_time, size_t capacity);
            ~TimerContainer();

            int init();

            

            long schedule (const TimerHandler &handler,
                           const time_t &future_time,
                           const time_t &interval = 0,
                           void* param = NULL);

            int reset_interval (long timer_id, const time_t &interval) ;
            int cancel (long timer_id, int dont_call_handle_close = 1) ;
            int expire (const time_t &current_time);

            uint32_t is_empty ();
            void poll();

        private:
            void check_timer(Timer* timer);
            unsigned long next_timer_id();
            long schedule_i (Timer* timer);
            int cascade(tvec_t *tv, int index);
            unsigned long INDEX(int N);

        private:
            size_t            _capacity;
            time_t            _base_time;
            tvec_base_t       _tvec_base;
            IDTimerMap        _id_timer;
            TimerPool         _timer_pool;
            uint32_t          _timer_count;
            list<Timer*>      _cancel_timer_list;
        };
    }
}

#endif
