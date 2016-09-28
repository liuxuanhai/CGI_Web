#pragma once

#include <cstdint>
#include <stack>
#include <unistd.h>
#include <ucontext.h>
#include <iostream>
#include <cassert>
#include <sys/mman.h>

typedef void* (*CoFunc)(int64_t coroutine_id, void* ptr_usr1, void* ptr_usr2);

struct Coroutine
{
    void* ptr_usr1;
    void* ptr_usr2;
    int64_t coroutine_id;
    int32_t status;
    ucontext_t ctx;
    void* yield;
    CoFunc func;
    void* stack;
};

class CoroutinePool
{
public:
    CoroutinePool() : memory_(NULL), capacity_(0), unit_page_num_(0), page_size_(0) {}

    static CoroutinePool& Instance()
    {
        static CoroutinePool pool;
        return pool;
    }

    bool Init(size_t capacity, size_t stack_size)
    {
        capacity_ = capacity;
        stack_size_ = stack_size;
        page_size_ = getpagesize();

        CalculateUnitPageNum();
		
        memory_ = mmap(NULL, capacity * UnitSize(), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (memory_ == MAP_FAILED)
        {
            std::cout << "CoroutinePool mmap failed" << std::endl;
            return false;
        }

        for (size_t i = 0; i < capacity; ++i)
        {
            free_list_.push(i);
            int r1 = mprotect(UnitAddress(i), page_size_, PROT_NONE);
            int r2 = mprotect(UnitAddress(i+1) - page_size_, page_size_, PROT_NONE);
            assert(r1 == 0 && r2 == 0);
        }

        return true;
    }

    void Release()
    {
        munmap(memory_, capacity_ * UnitSize());
        memory_ = NULL;
        capacity_ = 0;
        unit_page_num_ = 0;
        page_size_ = 0;
        free_list_ = std::stack<size_t>();
    }

    Coroutine* New()
    {
        if (free_list_.empty()) return NULL;
        return NewCoroutine(free_list_.top());
    }

    void Delete(Coroutine* c)
    {
        if (c) DeleteCoroutine(c);
    }

    void Dump() const
    {
        std::cout << "coroutine pool dump begin" << std::endl;
        std::cout << "memory address:" << memory_ << std::endl;
        std::cout << "capacity:" << capacity_ << std::endl;
        std::cout << "unit page num:" << unit_page_num_ << std::endl;
        std::cout << "page size:" << page_size_ << std::endl;
        std::cout << "free list size:" << free_list_.size() << std::endl;
        std::cout << "coroutine pool dump end" << std::endl;
    }

private:
    void CalculateUnitPageNum()
    {
        size_t size = sizeof(Coroutine) + stack_size_;
        unit_page_num_ = size / page_size_ + 2; //two protect pages
        if (size % page_size_) ++unit_page_num_;
    }

    Coroutine* NewCoroutine(unsigned index)
    {
        char* stack = UnitAddress(index) + page_size_;
        char* p = stack + stack_size_;
        Coroutine* c = new (p) Coroutine;
        c->stack = stack;
        free_list_.pop();
        return c;
    }

    void DeleteCoroutine(Coroutine* c)
    {
        char* p = (char*)c - stack_size_ - page_size_;
        size_t index = UnitIndex(p);
        assert(index != (size_t)-1);
        c->~Coroutine();
        free_list_.push(index);
    }

    size_t UnitIndex(void* p) const
    {
        if (p < memory_) return -1;
        ptrdiff_t d = (char*)p - (char*)memory_;
        if (d % UnitSize()) return -1;
        size_t index = d / UnitSize();
        if (index >= capacity_) return -1;
        return index;
    }

    char* UnitAddress(size_t index) const
    {
        return (char*)memory_ + index * UnitSize();
    }

    size_t UnitSize() const
    {
        return unit_page_num_ * page_size_;
    }

private:
    void* memory_;
    size_t capacity_;
    size_t stack_size_;
    size_t unit_page_num_;
    size_t page_size_;
    std::stack<size_t> free_list_;
};
