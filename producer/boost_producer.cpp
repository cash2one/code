#include <pthread.h>
#include <iostream>
#include <string>
#include <boost/function.hpp>
#include <boost/bind.hpp>

using namespace std;

class b_thrd
{
typedef boost::function<void()> thrd_func;
public:
    b_thrd(const thrd_func& func_, const string& thrd_name_);
    void create_thrd();
private:
    static void* run_thrd(void* arg);
private:
    pthread_t       m_tid;          // 线程id
    b_thrd_func     m_func;         // 外部调用函数
    string          m_thrd_name;    // 线程名
}; 

b_thrd::b_thrd(const thrd_func& func_, const string& thrd_name_):
    m_func(func_), m_thrd_name(thrd_name_){}

void b_thrd::create_thrd()
{
    if(create_pthread(m_tid&, NULL, &run_thrd, this) != 0)
        cout << "create_pthread error" << endl;
}

void* b_thrd::run_thrd(void* arg)
{
    b_thrd* thrd = static_cast<b_thrd*>(arg); 
    thrd->m_func();
    return NULL;
}

void prod_func()
{

}

void coms_func()
{

}

int main()
{
    boost::function<void()> func;

    func = boost::bind(&prod_func);
    b_thrd prod_thrd(&func, "producer");
    func = boost::bind(&coms_func);
    b_thrd coms_thrd(&func, "comsumer");

    prod_thrd.create_thrd();
    coms_thrd.create_thrd(); 

    return 0;
}
