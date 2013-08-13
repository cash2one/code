bind2nd(less<int>(), 0) (x)       x < 0 
// 只能绑定class,所以必须与仿函数(函数对象)一起
template <class Operation, class T>
binder2nd<Operation> bind2nd (const Operation& op, const T& x)
{
    return binder2nd<Operation>(op, typename Operation::second_argument_type(x));       // 返回值是bind2nd类 获取到返回值后再调用其仿函数
};

template <class Operation> 
class binder2nd:public unary_function <typename Operation::first_argument_type, typename Operation::result_type>
{
protected:
    Operation op;
    typename Operation::second_argument_type value;
public:
    binder2nd (const Operation& x, const typename Operation::second_argument_type& y) // 构造binder2nd类
        :op (x), value(y)
    {}

    typename Operation::result_type
    operator() (const typename Operation::first_argument_type& x) const     // 传入第一个参数
    {
        return op(x,value);         // 调用operation的仿函数
    }
};
