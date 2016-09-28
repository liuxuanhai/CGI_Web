#ifndef USE_HEAP_TIMER

#include "pqtimer.h"
#include "callback.h"
#include <cassert>
#include <cstdio>

static void generate_timer_id(int64_t& id)
{
    uint32_t low = id;
    uint32_t high = (id >> 32);
    id = ((int64_t(++high)) << 32) | low;
}

void PQTimer::TimerNode::Reuse()
{
    generate_timer_id(timer_id);
    expire_time.Reset();
    interval_time.Reset();
    cbo = NULL;
    data = NULL;
    delete_flag = false;
    occupied_flag = false;
}

int32_t PQTimer::TimerNode::Execute() const 
{ 
    return cbo->Execute(timer_id, data); 
}

bool PQTimer::TimerNode::TestExpire(const TimeValue& now) const 
{ 
    return now == expire_time || now > expire_time; 
}

PQTimer::PQTimer(uint32_t capacity)
    : capacity_(capacity)
{
    node_pool_.resize(capacity_);
    for (uint32_t i = 0; i < capacity_; ++i)
    {
        node_pool_[i].timer_id = i; //pool index assign to low32
        free_list_.push_back(&node_pool_[i]);
    }
}

int64_t PQTimer::RegisterTimer(const TimeValue& interval, const TimeValue& delay, CallbackObject* cbo, void* data)
{
    if (!interval) return -1;
    if (free_list_.empty()) return -1;

    auto node = free_list_.front();
    free_list_.pop_front();

    node->Reuse();
    node->expire_time = delay + time(NULL);
    node->interval_time = interval;
    node->data = data;
    node->cbo = cbo;
    node->occupied_flag = true;

    node_list_.push(node);

    return node->timer_id;
}

#define TimerIdToPoolIndex(timer_id) ((uint32_t)(timer_id))

void PQTimer::UnregisterTimer(int64_t timer_id) 
{
    unsigned pool_index = TimerIdToPoolIndex(timer_id);
    assert(pool_index < capacity_);
    node_pool_[pool_index].delete_flag = true;
}

int32_t PQTimer::Update(const TimeValue& now) 
{
    int ret = -1;

    while (!node_list_.empty())
    {
        auto node = node_list_.top();

        if (!node->TestExpire(now)) break;

        node_list_.pop();

        if (!node->delete_flag)
        {
            ret = 0;
            if (node->Execute() == 0)
            {
                //add again
                node->expire_time = now + node->interval_time;
                node_list_.push(node);
                continue;
            }
        }

        node->delete_flag = false;
        node->occupied_flag = false;
        free_list_.push_back(node);
    }

    return ret;
}

template <typename TimerNode>
static void PrintNode(std::ostringstream& ss, TimerNode* node, int id)
{
        ss << "PQTimer[" << id << "] "
            << "timer_id[" << node->timer_id << "] "
            << "expire_time[" << TimeValue::TimeName(node->expire_time.Sec()) << "] "
            << "interval_time[" << node->interval_time.Sec() << "] "
            << "CallbackObject[" << node->cbo->name() << "]\n";
}

void PQTimer::Print() const
{
    std::ostringstream ss;

    auto size = GetTimerSize();
    if (size > 0)
    {
        int id = 0;

        //占用NodeTimer的比较少的话，就不遍历POOL了
        if (size < 0.1 * capacity_)
        {
            auto pq(node_list_);
            while (!pq.empty())
            {
                auto node = pq.top();
                pq.pop();
                if (!node->delete_flag)
                    PrintNode(ss, node, ++id);
            }
        }
        else
        {
            for (uint32_t i = 0; i < capacity_; i++) 
            {
                auto node = &node_pool_[i];
                if (node->occupied_flag && !node->delete_flag) 
                    PrintNode(ss, node, ++id);
            }
        }
    }

    printf("PQTimer dump capacity[%u] \n%s", capacity_, ss.str().c_str());
}

#endif
