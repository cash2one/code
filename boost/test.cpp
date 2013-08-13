#include <iostream>

using namespace std;

struct F2
{
    int s;

    typedef void result_type;
    void operator()( int x ) { s += x; }
};

F2 f2 = { 1 };


int main()
{
   cout << f2.s << endl; 
}

//int a[] = { 1, 2, 3 };

//std::for_each( a, a+3, bind( ref(f2), _1 ) );

//assert( f2.s == 6 );
