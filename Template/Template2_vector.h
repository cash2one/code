#include <iostream>
#include <stdexcept>

using namespace std;

template <class T> class vector_t
{
private:
    T* data;
    T* avail;
    T* limit;
    int size_max;
public:
    vector_t():data(NULL),avail(NULL),limit(NULL),size_max(2)
    {
    };

    vector_t(int n, const T& u)
    {
        try{
            T* tmp = new T[n*2];
            if(tmp == NULL)
            {
                cerr << "new error" << endl;
            }
            data = tmp;
            for(int i = 0;i < n; i++)
            {
                *tmp++ = u;
            }            
            avail = data + n;
            limit = data + 2 * size_max;
            size_max = 2 * n;
        }catch(exception &x){
            cerr << x.what() << endl;
        }

    };

    vector_t(const vector_t& other):data(NULL),avail(NULL),limit(NULL),size_max(2)
    {
        T* tmp = other.data;
        
        while(tmp != other.avail)
        {
            push_back(*tmp++);
        }
    };

    ~vector_t()
    {
        if(data != NULL)
        {
            delete[] data; 
        }
        cout << "destory success" << endl;
    };

    vector_t& operator= (const vector_t& other)
    {
        T* tmp = other.data;
        clear();        
        while(tmp != other.avail)
        {
            push_back(*tmp++);
        }
        return *this;
    }
    
    const T& operator[] (int x)
    {
        return *(data+x);
    }
    
    const T& front()
    {
        return *data;
    }

    const T& back()
    {
        return *(avail - 1);
    }
    
    int size()
    {
        return avail - data;
    }

    void clear()
    {
        if(data != NULL)
        {
            delete[] data;
            avail = NULL;
            limit = NULL;
            size_max = 2;
        }
        else 
        {
            cout << "Not Found" << endl;
            return;
        }
    }

    void erase(T* position)
    {
        if(position == NULL)
        {
            return ;
        }
        
        T* tmp(position);
        while(tmp != end())
        {
            *tmp++ = *(tmp+1);
        }
        avail--;
    }

    void erase(T* first,T* last)
    {
        if(first == NULL || last == NULL)
        {
            return ;
        }
        T* tmp(first);
        T* tmp_end(last);

        while(tmp_end != avail)
        {
            *tmp++ = *tmp_end++;
        }
        avail = avail - (last - first);
    }

    bool empty()
    {
        return (avail == NULL)? true:false;
    }

    T* begin()
    {
        return data;
    }

    T* end()
    {
        return avail;
    }

    const T& at(int n)
    {
        return *(data+n);
    }

    int capacity()
    {
        return limit - data;
    }

    void push_back(const T& x)
    {
        if(avail == NULL)
        {
            T* tmp = new T[size_max];
            data = tmp;
            avail = tmp;
            limit = tmp + 1;
            *avail++ = x;
        }
        else
        {
            if(avail == limit)
            {
                T* new_buf = new T[2*size_max];
                copy(data,data + size_max,new_buf);
                avail = new_buf + size_max - 1;
                *avail = x;
                limit = data + 2*size_max;  
                delete[] data;
                data = new_buf;
                avail = data + size_max;
                size_max *= 2;        
            }
            else
            {
                *avail++ = x;
            }
        }         
    }

    void pop_back()
    {
        erase(avail-1);
    }
    
    void insert(T* position, const T& x)
    {
        T* tmp(avail);
        if(position > avail || position < data)
        {
            throw overflow_error("pointer error");
            return ;
        }
        push_back(*avail);
        while(tmp != position)
        {
            *tmp = *(tmp-1);
            tmp--;
        }
        *position = x;
    }

    T* insert(T* position, int n, const T& x)
    {
        if(position > avail || position < data)
        {
            throw overflow_error("pointer error");
            return ;
        }
        for(int i = 0; i < n; i++)
        {
            insert(position,x);
        }
    }

    T* insert(T* position, T* first, T* last)
    {
        T* tmp(first);
        T* tmp_pos(position);

        if(position > avail || position < data)
        {
            throw overflow_error("pointer error");
            return ;
        }
        if(first == NULL || last == NULL)
        {
            cout << "input pointer error" << endl;
            return;
        }
        while(tmp != last)
        {
            insert(tmp_pos++,*tmp++);
        }   
    }

    void assign(int n, const T& u)
    {
        clear();
        for(int i = 0; i < n; i++)
        {
            push_back(u);
        }
    }
};
