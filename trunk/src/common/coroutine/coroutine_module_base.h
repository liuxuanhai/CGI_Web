#ifndef _COROUTINE_MODULE_BASE_H_
#define _COROUTINE_MODULE_BASE_H_

#include "comm_def.h"
#include "callback.h"
#include "heap_timer.h"
//#include "utils.h"
#include "coroutinepool.h"
#include <assert.h>
#include "common_util.h"

#include <pthread.h>

extern log4cplus::Logger 				  coLogger;

enum CoroutineHandlerType
{
    CREATE_CO,
    RESUME_CO
};

typedef struct CoroutineHandlerMeta
{
    int32_t type;
    CoFunc func;
    time_t task_delay_secs;
}CoroutineHandlerMeta;

enum CoroutineStatus
{
    CO_READY,
    CO_SUSPENDED,
    CO_RUNNING,
    CO_FINISHED
};


template<class CoroutineModuleBase>
inline void* Yield(void* ptr = NULL)
{
    // FuncCostMonitor;
    CoroutineModuleBase& co_module = Singleton<CoroutineModuleBase>::Instance();
    if (co_module.running() < 0)
        return 0;

    int64_t cur_co_id = co_module.running();
    ASSERT(cur_co_id != 0, "cur_co_id == 0, can't use Yield");

    co_module.set_running(-1);

    Coroutine* co = co_module.GetCoroutine(cur_co_id);
    ASSERT(co != NULL, "GetCoroutine(running = %ld) == NULL", cur_co_id);

    // 通过co->yield给主进程传递参数, 也就是主进程Resume的返回值
    co->yield = ptr;
    co->status = CO_SUSPENDED;

    swapcontext(&co->ctx, co_module.main_context());

    void* ret = co->yield;
    co->yield = NULL;
    return ret;
}

//template<class APP, class CONFIGMODULE, class TIMERMODULE>
template<class TIMERMODULE>
class CoroutineModuleBase : public CallbackObject
{
    public:
        CoroutineModuleBase()
			: msg_id_handling_(0) 
		{
			if(pthread_mutex_init(&_mutex, NULL) != 0)
				assert(0);
		};
        virtual ~CoroutineModuleBase() 
        {
			pthread_mutex_destroy(&_mutex);
        };

        void ReadLock()
        {
			if(pthread_mutex_lock(&_mutex) != 0)
				assert(0);
        }

        void ReleaseLock()
        {
			if(pthread_mutex_unlock(&_mutex) != 0)
				assert(0);
        }

        int32_t BaseInit(int capacity, size_t stack_size)			
        {
			capacity_ = capacity;
			stack_size_ = stack_size;
			
            coroutine_timerout_callback_.SetHandler(this, &CoroutineModuleBase::OnCoroutineTimeout);
            coroutine_timerout_callback_.set_name("CoroutineModuleBase coroutine_timerout_callback");

            running_ = -1;

            runtime_print_callback_.SetHandler(this, &CoroutineModuleBase::OnRuntimePrint);
            runtime_print_callback_.set_name("CoroutineModuleBase runtime_print_callback");
            
            if (!CoroutinePool::Instance().Init(capacity_, stack_size_))
            {
                LOG_ERROR("CoroutinePool Init Failed\n");
                return -1;
            }

            return 0;
        }

        void BaseFini()
        {
            for (CoroutineMap::iterator it = coroutine_map_.begin(); it != coroutine_map_.end(); ++it) 
                MemFree(it->second);
            coroutine_map_.clear();
            coroutine_handler_meta_map_.clear();
            CoroutinePool::Instance().Release();
        }

        virtual int32_t PreCreate(Coroutine* co)
        {
            (void)co;
            return 0;
        }
        virtual int32_t PreResume(Coroutine* co)
        {
            (void)co;
            return 0;
        }
        virtual int32_t PreDel(Coroutine* co)
        {
            (void)co;
            return 0;
        }

    public:
        Coroutine* GetCoroutine(int64_t coroutine_id)
        {
            CoroutineMap::const_iterator it = coroutine_map_.find(coroutine_id);
            if (it == coroutine_map_.end()) 
            {
                LOG_ERROR("can't find coroutine_id[%ld]", coroutine_id);
                return NULL;
            }
            return it->second;
        }

        virtual void* InvokeCoroutineFunction(Coroutine* co)
        {
            return co->func(co->coroutine_id, co->ptr_usr1, co->ptr_usr2);
        }

        static void Schedule(uint32_t low32, uint32_t hi32)
        {
            // FuncCostMonitor;
            uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
            CoroutineModuleBase* co_module = (CoroutineModuleBase*)ptr;

            int64_t running = co_module->running();

            // 获取正在运行的协程
            Coroutine* co = co_module->GetCoroutine(running);
            ASSERT(co != NULL, "GetCoroutine(running = %ld) == NULL", running);
            LOG_DEBUG("coroutine stack start at %p, ss_sp = %p, ss_size = %lu, start-ss_sp = %lu",
                &ptr, co->ctx.uc_stack.ss_sp, co->ctx.uc_stack.ss_size,
                (uint64_t)(&ptr) - uint64_t(co->ctx.uc_stack.ss_sp));

            // 执行函数， 通过co->yield传递协程函数返回值
            co->yield = co_module->InvokeCoroutineFunction(co);

            // 到这步表示整个协程函数执行完毕
            co_module->set_running(-1);
            co->status = CO_FINISHED;
        }

        Coroutine* MemAlloc()
        {
            return CoroutinePool::Instance().New();
        }

        void MemFree(Coroutine* c)
        {
            CoroutinePool::Instance().Delete(c);
        }

        int64_t CreateCoroutine(CoFunc func,  time_t task_delay_secs, void* ptr_usr1, void* ptr_usr2)
        {
            // FuncCostMonitor;
            // 等待时间为0为即时任务, 不需要注册定时器
			int64_t coroutine_id = 0;            
						
            if (task_delay_secs > 0) 
            {
                TIMERMODULE& timer_module = Singleton<TIMERMODULE>::Instance();
                HeapTimer* heap_timer = timer_module.GetHeapTimer();
                coroutine_id = heap_timer->RegisterTimer(TimeValue(task_delay_secs), TimeValue(task_delay_secs), &coroutine_timerout_callback_, NULL);
            }

            Coroutine* co = MemAlloc();
            if (co == NULL) 
            {
                LOG_ERROR("MemAlloc Coroutine ERROR");
                return -1;
            }

            // 传递参数
            co->ptr_usr1= ptr_usr1;
            co->ptr_usr2= ptr_usr2;
            co->coroutine_id = coroutine_id;
            co->func = func;
            co->yield = NULL;
            co->status = CO_READY;

            if (PreCreate(co) != 0) 
            {
                LOG_DEBUG("PreCreate error!");
                MemFree(co);
                return -1;
            }

            // 插入coroutine_map_进行管理
            coroutine_map_.insert(CoroutineMap::value_type(coroutine_id, co));
            LOG_DEBUG("coroutine_map_ insert coroutine_id: %ld", coroutine_id);

            return coroutine_id;
        }

        void* Resume(int64_t coroutine_id, void* ptr = NULL)
        {
            // FuncCostMonitor;
            Coroutine* co = GetCoroutine(coroutine_id);
            if (co == NULL || co->status == CO_RUNNING) 
            {
                LOG_DEBUG("coroutine_id[%ld] is not exist! maybe not need to rsp OR rsp is timeout", coroutine_id);
                return NULL;
            }

            if (PreResume(co) != 0) 
            {
                LOG_DEBUG("PreResume error!");
                return NULL;
            }

            // 通过co->yield从主进程传递参数给协程
            co->yield = ptr;
            int ucontext_ret = 0;
            switch (co->status) 
            {
                case CO_READY:
                    {
                        // 刚创建的协程，需要创建context
                        ucontext_ret = getcontext(&co->ctx);
                        ASSERT(ucontext_ret == 0, "getcontext error");

                        co->status = CO_RUNNING;
                        co->ctx.uc_stack.ss_sp = co->stack;
                        co->ctx.uc_stack.ss_size = stack_size_;
                        co->ctx.uc_link = &main_context_;

                        running_ = coroutine_id;
                        uintptr_t ptr = (uintptr_t)this;
                        // makecontxt只支持32位int的参数传递，所以分了2个参数传递指针
                        makecontext(&co->ctx, (void (*)())Schedule, 2, (uint32_t)ptr, (uint32_t)(ptr>>32));
                        ucontext_ret = swapcontext(&main_context_, &co->ctx);
                        ASSERT(ucontext_ret == 0, "swapcontext error");
                    }
                    break;
                case CO_SUSPENDED:
                    {
                        running_ = coroutine_id;
                        co->status = CO_RUNNING;
                        ucontext_ret = swapcontext(&main_context_, &co->ctx);
                        ASSERT(ucontext_ret == 0, "swapcontext error");
                    }
                    break;
                default:
                    ASSERT(NULL, "Resume co->status = %d, NOT CO_READY or CO_SUSPENDED", co->status);
            }

            // 到这一步是协程函数结束或者协程被Yield
            // 通过co->yield传递协程中的return结果或者调用Yield的参数
            void* ret = co->yield;
            co->yield = NULL;

            // 如果结束释放定时器和Coroutine
            if (running_ == -1 && co->status == CO_FINISHED) 
            {
                DeleteCoroutine(coroutine_id);
				#if 1
                if (coroutine_id != 0) 
                {
                    TIMERMODULE& timer_module = Singleton<TIMERMODULE>::Instance();
                    HeapTimer* heap_timer = timer_module.GetHeapTimer();
                    heap_timer->UnregisterTimer(coroutine_id);
                }
				#endif
            }

            return ret;
        }

        void* RunCoroutine(CoFunc func,  time_t task_delay_secs, void* ptr_usr1, void* ptr_usr2)
        {
            int64_t coroutine_id = CreateCoroutine(func, task_delay_secs, ptr_usr1, ptr_usr2);
            if (coroutine_id <= 0) 
            {
                LOG_ERROR("CreateCoroutine coroutine_id = %ld error", coroutine_id);
                return NULL;
            }
            return Resume(coroutine_id);
        }

        void DeleteCoroutine(int64_t coroutine_id)
        {
            // FuncCostMonitor;
            Coroutine* co = GetCoroutine(coroutine_id);
            if (co == NULL) return;

            PreDel(co);
            MemFree(co);
            coroutine_map_.erase(coroutine_id);
        }

        int32_t OnCoroutineTimeout(int64_t coroutine_id, void* data)
        {
            (void)data;			
            LOG_DEBUG("OnCoroutineTimeout: coroutine_id[%ld] TIMEOUT!!!", coroutine_id);

			ReadLock();
			Resume(coroutine_id, NULL);
			ReleaseLock();
            //DeleteCoroutine(coroutine_id);

            return -1;
        }

        int32_t OnRuntimePrint(int64_t timer_id, void* data)
        {
            std::ostringstream str_map_info;
            for (CoroutineMap::iterator it = coroutine_map_.begin(); it != coroutine_map_.end(); ++it) 
            {
                str_map_info << "\tcoroutine_id[" << it->first << "] status[" << it->second->status << "]\n";
            }
            return 0;
        }

        int32_t AddRuntimePrintTimer()
        {
        	#if 1
            //CONFIGMODULE& conf_module = Singleton<CONFIGMODULE>::Instance();
            TIMERMODULE& timer_module = Singleton<TIMERMODULE>::Instance();
            HeapTimer* heap_timer = timer_module.GetHeapTimer();
            int64_t timer_id = heap_timer->RegisterTimer(TimeValue(5), TimeValue(0), &runtime_print_callback_, NULL);
            if (timer_id > 0) 
            {
                LOG_DEBUG("register runtime print timer ok!");
                return 0;
            }

            return -1;
			#endif 

			return 0;
        }

        inline const ucontext_t* main_context() const
        {
            return &main_context_;
        }

        void Register(int32_t msg_id, int32_t type, CoFunc func = NULL, time_t task_delay_secs = 0)
        {
            CoroutineHandlerMeta coroutine_handler_meta;
            coroutine_handler_meta.type = type;
            coroutine_handler_meta.func = func;
            coroutine_handler_meta.task_delay_secs = task_delay_secs;
            coroutine_handler_meta_map_[msg_id] = coroutine_handler_meta;
        }

        void PrintRegisterCmd()
        {
            for (CoroutineHandlerMetaMap::iterator it = coroutine_handler_meta_map_.begin(); it != coroutine_handler_meta_map_.end(); ++it) 
            {
                LOG_DEBUG("CoroutineModue Register cmd[%d] type[%d]", it->first, it->second.type);
            }
        }

        template<typename T>
        int32_t HandleRequest(const char* pkg_buf, const size_t pkg_len, void* data)
        {
            T msg;

            // Protobuf解析
            if (!msg.ParseFromArray(pkg_buf, pkg_len)) 
            {
                LOG_ERROR("protobuf parse error!");
                return -1;
            }

            return HandleRequest<T>(msg, data);
        }

        template<typename T>
        int32_t HandleRequest(const T& msg, void* ptr_usr2)
        {
            // 根据cmd进行处理
            CoroutineHandlerMetaMap::iterator it = coroutine_handler_meta_map_.find(msg.head().cmd());
            if (it != coroutine_handler_meta_map_.end()) 
            {
                msg_id_handling_ = msg.head().cmd();

                // 处理消息
                PrintPkg(msg, "recv msg");
                CoroutineHandlerMeta coroutine_handler_meta = it->second;
                if (coroutine_handler_meta.type == CREATE_CO) 
                {
                    int64_t coroutine_id = CreateCoroutine(coroutine_handler_meta.func, coroutine_handler_meta.task_delay_secs, (void*)&msg, ptr_usr2);
                    if (coroutine_id < 0) 
                    {
                        LOG_ERROR("CreateCoroutine coroutine_id = %ld", coroutine_id);
                        return -1;
                    }
                    Resume(coroutine_id);
                } 
                else if (coroutine_handler_meta.type == RESUME_CO) 
                {
                    int64_t coroutine_id = msg.head().seq();
                    if (coroutine_id > 0) 
                    {
                        Resume(coroutine_id, (void*)&msg);
                    }
                }
            } 
            else 
            {
                LOG_ERROR("msg[%d] is not register!", msg.head().cmd());
                return -2;
            }

            return 0;
        }

        virtual bool NeedPrintPkg(uint32_t cmd)
        {
            UNUSE_ARG(cmd);
            return true;
        }

        template<typename T>
        void PrintPkg(const T& msg, const char* text = NULL)
        {
            (void)text;
            bool need_print = NeedPrintPkg(msg.head().cmd());
            LOG_DEBUG("msg_size = %d", msg.ByteSize());
            if (need_print) 
            {
                LOG_DEBUG("%s\n%s", text, msg.Utf8DebugString().c_str());
            } 
            else 
            {
                const google::protobuf::Descriptor* msg_dp = msg.GetDescriptor();
                if (msg_dp == NULL)
                    return;
                const google::protobuf::FieldDescriptor* field_dp = msg_dp->FindFieldByNumber(msg.head().cmd());
                if (field_dp == NULL)
                    return;
                LOG_DEBUG("%s\nhead {\n%s\n}\n%s", text, msg.head().Utf8DebugString().c_str(), field_dp->lowercase_name().c_str());
            }
        }

        template<typename T>
        const char* GetBodyName(const T& msg)
        {
            const google::protobuf::Descriptor* msg_dp = msg.GetDescriptor();
            if (msg_dp == NULL)
                return NULL;
            const google::protobuf::FieldDescriptor* field_dp = msg_dp->FindFieldByNumber(msg.head().cmd());
            if (field_dp == NULL)
                return NULL;
            return  field_dp->lowercase_name().c_str();
        }

    private:
        typedef std::map<int64_t, Coroutine*> CoroutineMap;

        SYNTHESIZE(int64_t, running); // 正在运行的coroutine, -1表示没有
        ucontext_t          main_context_;
        CoroutineMap        coroutine_map_;

		int capacity_;
        size_t stack_size_;

        CallbackObject      coroutine_timerout_callback_;
        CallbackObject      runtime_print_callback_;

        typedef std::map<int32_t, CoroutineHandlerMeta> CoroutineHandlerMetaMap;
        CoroutineHandlerMetaMap coroutine_handler_meta_map_;

        pthread_mutex_t _mutex;

    protected:
        int msg_id_handling_;

        DISALLOW_COPY_AND_ASSIGN(CoroutineModuleBase);
};

#endif

