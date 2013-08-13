// wty:2013/08/12   boost_bind
//
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <boost/bind.hpp>

using namespace std;

// 1. common function 
int f(int a, int b)
{
    return a + b;
} 

std::bind1st(std::ptr_fun(f), 5)(x);   // f(5, x)
bind(f, 5, _1)(x);                     // f(5, x)

// 2. member function 
struct X
{
    bool f(int a);
};
X x;

shared_ptr<X> p(new X);
int i = 5;

bind(&X::f, ref(x), _1)(i);     // x.f(i)
bind(&X::f, &x, _1)(i);         // (&x)->f(i)
bind(&X::f, x, _1)(i);          // (internal copy of x).f(i)
bind(&X::f, p, _1)(i);          // (internal copy of p)->f(i)

// 3. function object 
struct F
{
    int operator()(int a, int b) { return a - b; }
    bool operator()(long a, long b) { return a == b; }
};
F f;

int x = 104;
bind<int>(f, _1, _1)(x);        // f(x, x), i.e. zero

int x = 8;
bind(std::less<int>(), _1, 9)(x);   // x < 9

// 4. nested bind
bind(f, bind(g, _1))(x);               // f(g(x))

