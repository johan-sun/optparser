optparser
===

optparser 是一个使用纯C实现的仿照argparse和boost program option API的c 命令行解解析库，整个命令行解析对象的生成非常方便，使用一个构建器即可链式生成。

##基本用法

###示列:

```
char const* gerater_then_zero(int n, void* context)
{
    if ( n <= 0 )
        return "must greater then zero";
    return NULL;
}

void print_cmd(void* ctx)
{
    char const* sub_cmd = *(char const**)ctx;
    printf("%s is sub command\n", sub_cmd);
}

int main(int argc, char** argv)
{
    int sigma;
    char const* sub_cmd = NULL;
    opt_init(argv[0])
            ->add("sigma,s", "special the sigma", opt_int(&sigma)->required()->validator(gerater_then_zero)->default_value(10))
            ->help("show help")
            ->more_help("command", "print help of sub command", opt_string(&sub_cmd), print_cmd, &sub_cmd)
            ->parse_into(argc, argv, &parser);
    printf("%d\n", sigma);
    if(sub_cmd)
    {
        free(sub_cmd);
    }
    opt_free(parser);
    return 0;
}
```
上述代码初始化了一个用程序名命名的解析器，拥有一个参数sigma，短：参数为s，拥有一个验证器必须大于0(验证器不需要上下文参数)，默认值为10，
定义了一个help，当解析命令行参数的时候出现--help活则-h理解现实格式化后的解析器，输出格式将仿照boost program options
函数`more_help`定义为注册一个自定义的可以带参数的帮助现显示命令，以支持显示子命令帮助，用户需要提供一个打印函数，可选的上下文，这里将注册的`sub_cmd`字符串作为上下文,以上几行c代码相当于下面的c++ program options代码。
```
#include  <iostream>
#include  <string>
#include  <boost/program_options.hpp>
using namespace std;

namespace po = boost::program_options;
int main(int argc, char *argv[])
{
    int sigma;
    string sub_cmd;
    po::options_description desc(argv[0]);
    desc.add_options()
        ("sigma,s", po::value(&sigma)->required()->default_value(10), "special the sigma")
        ("help,h", "show help")
        ("command", po::value(&sub_cmd), "show help of sub command");

    po::variables_map vm;
    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
    }
    catch ( po::error const& err )
    {
        cerr << err.what() << endl;
        return 1;
    }
    try
    {
        vm.notify();
    }catch (po::error const& err)
    {
        cerr << err.what() << endl;
        return 1;
    }

    if ( vm.count("help") )
    {
        cout << desc << endl;
        return 0;
    }
    if ( vm.count("command") )
    {
       cout << sub_cmd << " is sub cmd " << endl;
      return 0; 
    }

    if ( sigma < 0 )
    {
        cerr << "sigma must greator than zero " << endl;
        return 1;
    }
    cout << sigma << endl;
    return 0;
}
```

本库被设计为可以扩展，目前暂时提供了int,double,c 风格字符串，int数组,通过提供适当的`struct option_value`可以支持更多的格式化数据，诸如直接将参数中的字符串格式化填写到`struct sockaddr`, glib的`GList`, 以及其他常用clib的数据结构。

##本库还在开发中

文档将慢慢完善
