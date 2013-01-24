#include <stdio.h>
#include <iostream>

using std::cout; 
using std::endl;

namespace wty
{
template<class T> struct list_node
{
    list_node():prior(NULL),next(NULL){};
    list_node<T>* prior;
    list_node<T>* next;
    T data;
};

template<class T> class list
{
    list_node<T>* head;
    list_node<T>* tail;
    int size_cnt;
public:

    typedef void (*func_dis)(list_node<T>*);

    list():head(NULL),tail(NULL),size_cnt(0){};

    ~list(){};

    list(const list& other):head(NULL),tail(NULL),size_cnt(0)
    {
        list_node<T>* tmp = other.head;

        while(tmp)
        {
            push_back(tmp->data);
            tmp = tmp->next;
        }
    }

    list<T>& operator= (const list<T>& other)
    {
        if(this != &other)
        {
            list_node<T>* tmp = other.head;
            clear();
            while(tmp)
            {   
                push_back(tmp->data);
                tmp = tmp->next;
            }
        }

        return *this;
    }
  
    list_node<T>* push_front(const T& input)
    {
        list_node<T>* tmp = new list_node<T>;

        tmp->data = input;
        if(NULL != head)
        {   
            tmp->next = head;
            head->prior = tmp;
        }
        head = tmp;
        if(NULL == tail)
        {
            tail = head; 
        }
        size_cnt++;
        return tmp;
    }

    list_node<T>* push_back(const T& input)
    {
        list_node<T>* tmp = new list_node<T>;

        tmp->data = input;
        if(NULL != tail)
        {
            tmp->prior = tail;
            tail->next = tmp;
        }
        tail = tmp;
        if(NULL == head)
        {
            head = tail;
        }
        size_cnt++;
        return tmp;
    }

    list_node<T>* erase(list_node<T>* node_)
    {
        if(node_ == NULL)
        {
            return NULL;
        }
        list_node<T>* tmp(node_->next);
        if(head == tail)
        {
            head = NULL;
            delete node_;
        }
        else if(node_ == head)
        {
            node_->next->prior = NULL;
            head = head->next; 
            delete node_;
        }
        else if(node_ == tail)
        {
            node_->prior->next = NULL;
            tail = tail->prior;    
            delete node_;   
        }
        else
        {
            node_->next->prior = node_->prior;
            node_->prior->next = node_->next;
            delete node_;
        }
        size_cnt--;
        return tmp;
    }

    void erase(list_node<T>* node_begin,list_node<T>* node_end)
    { 
        if(node_begin == NULL || node_end == NULL)
        {
            return ;
        }
        list_node<T>* node_tmp(node_begin);
        list_node<T>* node_del(node_begin);

        while(node_tmp != node_end->next)
        {
            node_del = node_tmp;
            erase(node_del);
            node_tmp = node_tmp->next;
        }
    }

    void clear()
    { 
        list_node<T>* tmp(head);    
        list_node<T>* tmp_del(head);    

        while(tmp)              
        {
            tmp_del = tmp;
            erase(tmp_del); 
            tmp = tmp->next;
        }
    }

    void reverse()
    {
        list_node<T>* tmp(tail);
        list_node<T>* old_node(tail);

        std::swap(tail, head);
        while (tmp)
        {
            old_node = tmp->prior;              // memory tmp->prior before swap 
            std::swap(tmp->next, tmp->prior);
            tmp = old_node;
        }
    }

    void assign(list_node<T>* node_,const T& value)
    {
        if(node_ == NULL)
        {
            return ; 
        }
        node_->data = value;
    } 

    void dis_data(func_dis func)
    {
        if(func == NULL)
        {
            return ;
        }
        list_node<T>* tmp(NULL);

        if(head != NULL)
            tmp = head;
        else
            cout << "List is NULL." << endl;
        while(NULL != tmp)
        {
            func(tmp);
            tmp = tmp->next;
        }
    }

    void remove(const T& value)
    {
        list_node<T>* tmp(head);

        while(tmp)
        {
            if(tmp->data == value)
            {
                erase(tmp);
            }
            tmp = tmp->next;
        }
    }

    void remove_if(bool (*func)(const T&))
    {       
        if(*func == NULL)
        {
            return ; 
        }
        list_node<T>* tmp(head);

        while(tmp)
        {
            if(func(tmp->data))
            {
                erase(tmp);
            }
            tmp = tmp->next;
        }
    }

    int size()
    {   
        return size_cnt;
    }

    bool empty()
    {
        return (head == NULL) ? true:false;
    }

    void pop_back()
    {
        erase(tail);
    }

    void pop_front()
    {
        erase(head);
    }

    T& front()
    {
        return head->data;
    }

    const T& front() const
    {
        return head->data;
    }

    T& back()
    {
        return tail->data;
    }

    const T& back() const
    {
        return tail->data;
    }

    void merge(list<T> list_back)
    {
        if(list_back.head == NULL)
        {
            return ;
        }
        tail->next = list_back.head;
        list_back.head->prior = tail;
        tail = list_back.tail;          // tail redefine
    }

    void sort()                         
    {
        list_node<T>* tmp(head);
        T data_tmp;

        for(int i = 0;i < size();i++)       
        {
            while(tmp !=tail)
            {
                if(tmp->data > tmp->next->data)
                {
                    data_tmp = tmp->data;
                    tmp->data = tmp->next->data;
                    tmp->next->data = data_tmp;
                }
                tmp = tmp->next;
            }
            tmp = head;
        }
    }
    
    void unique()
    {
        list_node<T>* tmp_base(head);

        while(tmp_base)
        {
            list_node<T>* tmp_comp = tmp_base->next;  // local variable 
            while(tmp_comp)
            {
                if(tmp_comp->data == tmp_base->data)
                {
                    erase(tmp_comp);
                }
                tmp_comp = tmp_comp->next;
            }
            tmp_base = tmp_base->next;
        }
    } 

    void swap(list<T>& lst)
    { 
        list_node<T>* tmp(head);

        head = lst.head;
        lst.head = tmp;
        tmp = tail;
        tail = lst.tail;
        lst.tail = tmp;
    }
};
}
