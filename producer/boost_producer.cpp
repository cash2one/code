// wty: 2013/08/14 
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
    b_thrd(const thrd_func& func_, const string& thrd_name_, pthread_mutex_t& oper_lock_);
    void create_thrd();
    void lock_thrd(pthread_mutex_t& lock_);
    void unlock_thrd(pthread_mutex_t& lock_);
    string      get_thrd_name();
    pthread_t   get_tid();
private:
    static void* run_thrd(void* arg);
private:
    pthread_t       m_tid;          // 线程id
    thrd_func       m_func;         // 外部调用函数
    string          m_thrd_name;    // 线程名
    pthread_mutex_t m_oper_lock;    // 线程锁
}; 

b_thrd::b_thrd(const thrd_func& func_, const string& thrd_name_, pthread_mutex_t& oper_lock_):
    m_func(func_), m_thrd_name(thrd_name_), m_oper_lock(oper_lock_){}

void b_thrd::create_thrd()
{
    if(pthread_create(&m_tid, NULL, &run_thrd, this) != 0)
        cout << "pthread_create error. tid:" << m_tid << endl;
}

void b_thrd::lock_thrd(pthread_mutex_t& lock_)
{
    if(pthread_mutex_lock(&lock_) != 0)
        cout << "lock_thrd error. tid:" << m_tid << endl;
}

void b_thrd::unlock_thrd(pthread_mutex_t& lock_)
{
    if(pthread_mutex_unlock(&lock_) != 0)
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

    cout << "thrd_name:" << thrd->m_thrd_name << ", tid:" << thrd->m_tid << " is running." << endl;
    thrd->lock_thrd(thrd->m_oper_lock);
    thrd->m_func();
    thrd->unlock_thrd(thrd->m_oper_lock);

    return NULL;
}

boost::circular_buffer<int> cir_buf(10);
pthread_mutex_t g_empty_lock;
void cout_buf()
{
    cout << "buffer:" << endl;
    for(int i = 0; i < cir_buf.size(); i++)
        cout << cir_buf[i] << " ";
    cout << endl;
}

void prod_func()
{
    cir_buf.push_back(1);
    cout_buf();
    if(cir_buf.size() == 1)
        pthread_mutex_unlock(&g_empty_lock);
}

void coms_func()
{
    pthread_mutex_lock(&g_empty_lock);
    cir_buf.pop_front();            
    cout_buf();
    if(cir_buf.size() == 0)
        pthread_mutex_lock(&g_empty_lock);
}

int main()
{
    boost::function<void()> func;
    pthread_mutex_t oper_lock;
    pthread_mutex_init(&oper_lock, NULL);
    pthread_mutex_init(&g_empty_lock, NULL);
    pthread_mutex_lock(&g_empty_lock);

    func = boost::bind(&prod_func);
    b_thrd prod_thrd(func, "producer1", oper_lock);
    prod_thrd.create_thrd();
    b_thrd prod_thrd1(func, "producer2", oper_lock);
    prod_thrd1.create_thrd();
    b_thrd prod_thrd2(func, "producer3", oper_lock);
    prod_thrd2.create_thrd();

    func = boost::bind(&coms_func);
    //b_thrd coms_thrd(func, "comsumer1", oper_lock);
    //coms_thrd.create_thrd(); 
    b_thrd coms_thrd1(func, "comsumer2", oper_lock);
    coms_thrd1.create_thrd(); 

    while(1){}

    pthread_mutex_destroy(&oper_lock);
    pthread_mutex_destroy(&g_empty_lock);

    return 0;
}
