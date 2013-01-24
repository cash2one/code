#include <iostream>
#include "Template2_vector.h"

using std::cout;
using std::endl;
using wty::vector;

int main()
{ 
    vector<int> vt1;
    vector<int> vt2;

    vt1.push_back(2);
    vt2.push_back(3);
    //vt1.insert(vt1.begin(),3,22);
    //vt1.insert(vt1.begin()+1,vt2.begin()+1,vt2.begin()+3);
    //vt1.pop_back();
    //vt1.erase(vt1.begin()+3);
    //vt1.erase(vt1.begin(),vt1.begin()+3);    
    
    //cout << vt1.at(3) << endl; 
    //cout << vt1[4] << endl; 
    cout << "--------------------------------------------------"<< endl;
    for(int* it = vt2.begin(); it != vt2.end() ; ++it)
    {
        cout << *it << "\t";
    }
    cout << endl;
    cout << "--------------------------------------------------"<< endl;
    cout << "size=" << vt2.size() << endl;
    cout << "front="<< vt1.front() << endl;
    //cout << vt1.back() << endl;
    //cout << vt1.at(2) << endl;

/*
    vector<int> vec1(3,100);
    vector<int> vec2;
    vector<int>::iterator it;
    vec1.assign(4,200);
    for(int i=0;i<10;i++)
    {
        vec1.push_back(i);
    }
    cout << vec1.back() << endl;
    vec1.erase(vec1.begin(),vec1.begin()+3);
    
    vec1.assign(7,100);
    it = vec1.begin()+1;
    vec2.assign(it,vec1.end()-1);
    vec1.insert(vec1.begin()+2,123);
    for(it = vec1.begin(); it != vec1.end(); ++it)
    {
        cout << *it << endl;
    }
*/
    return 0;
}
