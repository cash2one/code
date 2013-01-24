#include <iostream>
#include <time.h>
#include <stdint.h>
#include "Template1_list.h"

using std::cout;
using std::endl;
using wty::list;
using wty::list_node;

list_node<int>* del_head_node(NULL);
list_node<int>* del_tail_node(NULL);

void printf_int(list_node<int>* node_)
{
    if(node_->prior != NULL)
    {
        cout << "prior=" << uint64_t(node_->prior)<< ",";
    }
    else
    {
        cout << "              ,";
    }
    cout << "mid=" << uint64_t(node_)<< ",";
    if(node_->next != NULL)
    {
        cout << "next=" << uint64_t(node_->next); 
    }
    else
    {
        cout << "             ";
    }
    cout << ",data=" << node_->data << endl; 
}

void find_del_node(list_node<int>* node_){
    if(del_head_node == NULL)
        del_head_node = node_;
    del_tail_node = node_;
}

bool single_digit(const int& value)
{
    return (value<4);
}

int main(){
    clock_t start_time = clock();
    list<int> list1;
    
    list1.push_back(1);   
    list1.push_back(2);
    list1.push_back(3);
    list1.push_back(4);   
    list1.push_back(5);
    list1.push_back(6);

    list<int> list2(list1);

    list2.push_back(4);
    list2.push_back(5);
    
    cout << "------------------------------------------------------------" << endl;
    //list1.dis_data(find_del_node);                        
    //list1.merge(list2);                                   // merge
    //list1.sort();                                         // sort
    //list1.assign(NULL,5);                        // assign
    //cout << list1.size() << endl;                         // size
    //cout << list1.empty() << endl;                        // empty
    //list1.reverse();                                      // reverse
    //list1.erase(NULL);                    // erase 
    //list1.erase(NULL,NULL);             // erase
    //list1.unique();                                       // unique
    //cout << "head=" << list1.front() << endl;             // front
    //cout << "tail=" << list1.back() << endl;              // back
    //list1.pop_front();                                    // pop_front
    //list1.pop_back();                                     // pop_back
    //list1.clear();                                        // clear
    //list1.swap(list2);                                    // swap
    //list1.remove(3);                                      // remove
    //list1.remove_if(single_digit);                        // remove_if
    list1 = list1;                                        // operator=
    list1.dis_data(printf_int);
    //list2.dis_data(printf_int);
    clock_t end_time = clock();
    cout << "RunTime = " << static_cast<double>(end_time - start_time)/CLOCKS_PER_SEC * 1000 << "ms" << endl;
    return 0;
}
