#pragma once

#include "callback.h"
#include "heap_timer.h"

extern log4cplus::Logger 				  coLogger; 

class Module
{
    public:
        virtual ~Module() {};
        virtual int32_t     ModuleInit() = 0;
        virtual void        ModuleFini() = 0;
        virtual const char* ModuleName() const = 0;
        virtual int32_t     ModuleRun() { return -1; };
};

class TimerModule : public Module
{
    public:
        TimerModule() : heap_timer_(NULL) {}

        int32_t ModuleInit()
        {
            heap_timer_ = new HeapTimer();
            runtime_print_callback_.SetHandler(this, &TimerModule::OnRuntimePrint);
            runtime_print_callback_.set_name("TimerModule runtime_print_callback");            

            LOG_DEBUG("%s init ok!", ModuleName());
            return 0;
        }

        void ModuleFini()
        {
            SAFE_DELETE(heap_timer_);
            LOG_DEBUG("%s fini completed!", ModuleName());
        }

        const char* ModuleName() const 
        { 
            return "TimerModule"; 
        }

        int32_t ModuleRun() 
        { 
            return heap_timer_->Update(TimeValue::Time()); 
        }

        int64_t RegisterTimer(const TimeValue& interval, const TimeValue& delay, CallbackObject* cbo, void* data = NULL)
        {
            return heap_timer_->RegisterTimer(interval, delay, cbo, data);
        }

        void UnregisterTimer(int64_t timer_id)
        {
            heap_timer_->UnregisterTimer(timer_id);
        }

        HeapTimer* GetHeapTimer() 
        { 
            return heap_timer_; 
        }

        int32_t GetTimerSize() const
        {
            return heap_timer_->GetTimerSize();
        }

        int32_t AddRuntimePrintTimer()
        {
            if (RegisterTimer(TimeValue(5), TimeValue(0), &runtime_print_callback_, NULL) > 0)
            {
                LOG_DEBUG("register runtime print timer ok!");
                return 0;
            }
            return -1;
        }

    private:
        int32_t OnRuntimePrint(int64_t timer_id, void* data)
        {
            heap_timer_->Print();
            return 0;
        }

        HeapTimer* heap_timer_;
        CallbackObject  runtime_print_callback_;

        DISALLOW_COPY_AND_ASSIGN(TimerModule);
};
