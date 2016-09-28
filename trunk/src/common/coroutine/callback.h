#ifndef _CALLBACK_H_
#define _CALLBACK_H_

#include <stdlib.h>
#include <new>

class CallbackBase
{
    public:
        virtual ~CallbackBase() {};
        virtual int32_t Execute(int64_t code, void* data) = 0;
};

template<class T>
class Callback : public CallbackBase
{
    public:
        typedef int32_t (T::*Func)(int64_t code, void* data);

        Callback(T* obj, Func func) : obj_(obj), func_(func) {}
        virtual ~Callback() {};

        int32_t Execute(int64_t code, void* data)
        {
            return (obj_->*func_)(code, data);
        }

    private:
        T* obj_;
        Func func_;
};

class CallbackObject
{
    public:
        CallbackObject() : callback_(NULL) { name_[0] = '\0'; }
        virtual ~CallbackObject()
        {
            if(callback_)
                callback_->~CallbackBase();
        }

        template<class T>
            void SetHandler(T* obj, typename Callback<T>::Func func)
            {
                callback_ = new((void*)storage_)Callback<T>(obj, func);
            }

        int32_t Execute(int64_t code, void* data)
        {
            return callback_->Execute(code, data);
        }

        inline const char* name() const { return name_; }
        inline void set_name(const char* name) { strncpy(name_, name, sizeof(name_)); }

    private:
        char            storage_[sizeof(Callback<CallbackBase>)];
        char            name_[128];
        CallbackBase*   callback_;
};

#endif //_CALLBACK_H_

