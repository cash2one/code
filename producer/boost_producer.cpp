#include <pthread.h>
#include <iostream>
#include <string>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/circular_buffer.hpp>

using namespace std;

class b_thrd
{
typedef boost::function<void()> thrd_func;
public:
    b_thrd(const thrd_func& func_, const string& thrd_name_, pthread_mutex_t& mutex_);
    void create_thrd();
    void lock_thrd();
    void unlock_thrd();
    string      get_thrd_name();
    pthread_t   get_tid();
private:
    static void* run_thrd(void* arg);
private:
    pthread_t       m_tid;          // 线程id
    thrd_func       m_func;         // 外部调用函数
    string          m_thrd_name;    // 线程名
    pthread_mutex_t m_mutex;        // 线程锁
}; 

b_thrd::b_thrd(const thrd_func& func_, const string& thrd_name_, pthread_mutex_t& mutex_):
    m_func(func_), m_thrd_name(thrd_name_), m_mutex(mutex_){}

void b_thrd::create_thrd()
{
    if(pthread_create(&m_tid, NULL, &run_thrd, this) != 0)
        cout << "pthread_create error. tid:" << m_tid << endl;
}

void b_thrd::lock_thrd()
{
    if(pthread_mutex_lock(&m_mutex) != 0)
        cout << "lock_thrd error. tid:" << m_tid << endl;
}

void b_thrd::unlock_thrd()
{
    if(pthread_mutex_lock(&m_mutex) != 0)
        cout << "unlock_thrd error. tid:" << m_tid << endl;
}

string b_thrd::get_thrd_name()
{
    return m_thrd_name;
}

pthread_t b_thrd::get_tid()
{
    return m_tid;
}

void* b_thrd::run_thrd(void* arg)
{
    b_thrd* thrd = static_cast<b_thrd*>(arg); 

    //cout << "thrd " << m_tid << " is running." << endl;
    //lock_thrd();
    thrd->m_func();
    //unlock_thrd();
    //pthread_exit(NULL);

    return NULL;
}

boost::circular_buffer<int> cir_buf(10);

void cout_buf()
{
    for(int i; i < cir_buf.max_size(); i++)
        cout << "buf = " << cir_buf[i] << " " << endl;
}

void prod_func()
{
    if(!cir_buf.full())
    {
        cir_buf.push_back();
    }
}

void coms_func()
{
    if(!cir_buf.empty())    
    {
        cir_buf.pop_front();            
    }
}

int main()
{
    boost::function<void()> func;
    pthread_mutex_t g_mutex;
    pthread_mutex_init(&g_mutex, NULL);

    func = boost::bind(&prod_func);
    b_thrd prod_thrd(func, "producer", g_mutex);
    //func = boost::bind(&coms_func);
    //b_thrd coms_thrd(&func, "comsumer", g_mutex);

    prod_thrd.create_thrd();
    //coms_thrd.create_thrd(); 
    
    pthread_mutex_destroy(&g_mutex);

    return 0;
}
