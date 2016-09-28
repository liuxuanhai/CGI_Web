#ifndef USE_HEAP_TIMER

#pragma once

#include <cstdint>
#include <vector>
#include <queue>
#include <deque>
#include "time_value.h"

class CallbackObject;

class PQTimer //shorthand of PriorityQueueTimer
{
        static const uint32_t DEFAULT_CAPACITY = 1024*16;

        struct TimerNode
        {
            int64_t timer_id;   //high32分配seqNo. low32存pool里的index
            TimeValue expire_time;
            TimeValue interval_time;
            CallbackObject* cbo;
            void* data;
            bool delete_flag;
            bool occupied_flag;

            void Reuse();
            int32_t Execute() const;
            bool TestExpire(const TimeValue& now) const;
        };

        struct TimerNodeCompare
        {
            bool operator() (const TimerNode* lhs, const TimerNode* rhs) const
            {
                return lhs->expire_time > rhs->expire_time;
            }
        };

    public:
        PQTimer(uint32_t capacity = DEFAULT_CAPACITY);

        int64_t RegisterTimer(const TimeValue& interval, const TimeValue& delay, CallbackObject*, void* data = NULL);
        void UnregisterTimer(int64_t timer_id);

        int32_t Update(const TimeValue& now);

        int32_t GetTimerSize() const { return capacity_ - free_list_.size(); }

        void Print() const;

    private:
        uint32_t capacity_;
        std::vector<TimerNode> node_pool_;
        std::priority_queue<TimerNode*, std::vector<TimerNode*>, TimerNodeCompare> node_list_;
        std::deque<TimerNode*> free_list_;
};

#endif
