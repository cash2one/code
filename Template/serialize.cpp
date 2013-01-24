#include <string>
#include <iostream>

using namespace std;

class serialize_t
{
public:
    serialize_t():m_data(NULL),m_size(0),new_size(0),m_src(NULL){}
    int serialize(char* data_, int size_);

    const char* data() const
    {
        return m_data;
    }

    int size() const
    {
        return m_size;
    }

private:
    char*               m_data; //! 0x99
    int                 m_size;
    int                 new_size;
    char*               m_src;
};

int serialize_t::serialize(char* data_, int size_)
{
    m_size += size_;
    
    if(m_size > new_size)
    { 
        if(new_size==0)
            new_size = size_*2;
        else
        {
            new_size *= 2;
            delete m_data;
        }
        cout << "m_size=" << m_size << "new_size=" << new_size << endl;
        char* buf = new char[new_size];
        copy(m_data,m_data+new_size,buf);
        cout << "1" << endl;
    }
    for(char* p = data_; p!= data_+ size_; ++p)
    { 
        cout << "OK!" << endl;
        *m_data++ = *p;
    }
    return 0;
}

int mem_cpy(void* src_, void* dest_, int size_)
{
    for(char *p = (char*)src_, *q = (char*)dest_;p != (char*)src_ + size_;++p,++q)
    {
        *q = *p;
    }
    return 0;
}

int mem_setting(char* data_add, char value, int size)
{
    while (size--)
    {
        (*data_add++) = value;
    }
    return 0;
}

void printf_data(char* data_out)
{
    for(char* p = data_out; p != data_out + strlen(data_out); ++p)
    {
        cout << *p << endl;
    } 
}

int main()
{
    int a = 11;
    double b = 1.3;
    char *c = "abcabcabcabc";

    serialize_t ser;
    //ser.serialize((char*)&a, sizeof(int));
    //ser.serialize((char*)&b, sizeof(double));
    ser.serialize(c, strlen(c));
    int size = ser.size();
    cout<<"OK!"<<endl;
    return 0;
}
