#include <pthread.h>
#include <iostream>
#include <string>
#include <boost/function.hpp>
#include <boost/bind.hpp>

using namespace std;

class Thread
{
    typedef boost::function<void()> ThreadFun;
    public:
    Thread(const ThreadFun& threadFun,const string& threadName = string());
    pid_t     getThreadId();
    string    getThreadName();
    int       start();

    private:
    static void* startThread(void* thread);

    private:
    pthread_t   m_thread;           //线程句柄
    pid_t       m_tid;              //线程ID
    string      m_strThreadName;    //线程名称
    bool        m_bStarted;         //线程是否启动
    ThreadFun   m_func;             //线程处理函数
};

Thread::Thread(const Thread::ThreadFun& threadFun, const string& threadName):
    m_func(threadFun), m_strThreadName(threadName)
{
}

int Thread::start()
{
    m_tid = pthread_create(&m_thread, NULL, &startThread, this);
    return 0;
}

void* Thread::startThread(void* obj)
{
    Thread* thread = static_cast<Thread*>(obj);
    thread->m_func();
    return NULL;
}

pid_t Thread::getThreadId()
{
    return m_tid;
};

string  Thread::getThreadName()
{
    return m_strThreadName;
}

void ThreadProcess()
{
    int count = 100;
    for (int i = 0; i < count; i++)
    {
        if (i % 10 == 0)    
            cout<<"\n";
        cout<<i<<"\t";
    }
}

int main()
{
    boost::function<void()> f;
    f = boost::bind(&ThreadProcess);    
    Thread thread(f, "ThreadTest");
    thread.start();
    sleep(1000*1000);
    return 0;
}
