#ifndef _SINGLETON_H_
#define _SINGLETON_H_

// 需要单例的情况
// 1 会被其他组件大量访问
// 2 只存在一个实例
// 3 当访问的时候必须存在
// 4 不同组建间也有依赖关系

// 静态局部变量实现注意:
// 1 单线程使用, 多线程不安全
// 2 Singleton内使用的静态变量是一个局部静态变量，
//   因此只有在Singleton的Instance()函数被调用时其才会被创建
// 3 生存期贯穿于整个程序生命周期，从程序启动开始直到程序执行完毕
// 4 返回引用而不是指针, 因为Singleton的生存期由非用户代码管理,
//   指针可以为NULL可以被delete, 采用引用更合适

template<typename T>
class Singleton
{
    public:
        static T& Instance();

    private:
        Singleton(Singleton const&) {};
        Singleton& operator=(Singleton const&) {};

    protected:
        Singleton() {};
        ~Singleton() {};
};

template<typename T>
T& Singleton<T>::Instance()
{
    static T instance;
    return instance;
}

#endif
