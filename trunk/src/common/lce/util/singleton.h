#ifndef _SINGLETON_H_
#define _SINGLETON_H_

using namespace std;

template <class C>
class singleton
{
public:
    static C *get_instance()
    {
        if (_instance == 0)
                _instance = new C();
        return _instance;
    }

protected:
    static C *_instance;
};

#define INIT_SINGLETON(T)  template <> T *singleton<T>::_instance = 0
#define GET_SINGLETON(T)   singleton<T>::get_instance()

#endif
