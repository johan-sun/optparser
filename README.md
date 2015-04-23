optparser
===

optparser 是一个使用纯C实现的仿照argparse和boost program option API的c 命令行解解析库，整个命令行解析对象的生成非常方便，使用一个构建器即可链式生成。

##基本用法

###示列:

```c
#include  <stdio.h>
#include    "optparser.h"

char const* gerater_then_zero(int n, void* context)
{
    if ( n <= 0 )
        return "must be greater then zero";
    return NULL;
}

void print_cmd(void* ctx)
{
    char const* sub_cmd = *(char const**)ctx;
    printf("%s is a sub command\n", sub_cmd);
}
int main(int argc, char *argv[])
{
    OptionParser parser;
    int sigma;
    double speed;
    char const* sub_cmd = NULL;
    int seq[4] = {0,1,2,3};
    int n = 4;
    opt_init("Example of optparser")
    ->group("basic arguments")
        ->add("sigma,s", "specify the sigma", opt_int(&sigma)->required()->validator(gerater_then_zero)->default_value(10))
        ->add("speed,v", "specify the speed", opt_double(&speed)->required()->default_value(.5))
        ->add("beta,b", "specify the beta", opt_int(NULL)->required()->default_value(0))
        ->add("sequence", "specify the sequence", opt_ints(seq, &n)->delimiters(",.|:")->default_value(seq, 4))
    ->group("features")
        ->add("c", "toggle c style", NULL)
        ->add("f", "toggle f style", NULL)
        ->add("x", "toogle x style", NULL)
    ->group("help command")
        ->help("show help")
        ->more_help("command", "print help of sub commands", opt_string(&sub_cmd), print_cmd, &sub_cmd)
    ->text("this show basic use of optparser,\nyou can add groups and pure text to optparser.")
    ->parse_into(argc, argv, &parser);

    printf("sigma=%d\n",sigma);
    printf("speed=%g\n", speed);
    printf("c style ?:%d\n", opt_has(parser, "c"));
    printf("f style ?:%d\n", opt_has(parser, "f"));
    printf("x style ?:%d\n", opt_has(parser, "x"));
    printf("arg of beta is:%s\n", opt_get_arg(parser, "beta"));
    printf("beta did not associate with a variable, so you need to parse it by youself\n");
    puts("the input sequene:");
    for(int i = 0; i < n; ++i)
    {
        printf("%d ", seq[i]);
    }
    putchar('\n');
    printf("remain argc=%d\n", parser->argc);
    printf("remain argv:\n");
    for(int i = 0; i < parser->argc; ++i)
        printf(" %s\n", parser->argv[i]);

    opt_free(parser);
    return 0;
}
```
####输出样例:
```bash
% ./a.out  
Example of optparser:

basic arguments:
  -s [ --sigma ] arg (=10)                specify the sigma
  -v [ --speed ] arg (=0.5)               specify the speed
  -b [ --beta ] arg (=0)                  specify the beta
  --sequence arg (=0,1,2,3)               specify the sequence

features:
  -c                                      toggle c style
  -f                                      toggle f style
  -x                                      toogle x style

help command:
  -h [ --help ]                           show help
  --command arg                           print help of sub commands

this show basic use of optparser,
you can add groups and pure text to optparser.
```

```
% ./a.out --command sub 
sub is a sub command
```

```
% ./a.out -xf 
sigma=10
speed=0.5
c style ?:0
f style ?:1
x style ?:1
arg of beta is:0
beta did not associate with a variable, so you need to parse it by youself
the input sequene:
0 1 2 3 
remain argc=0
remain argv:
```

```
% ./a.out --speed=50.0 remain arguments 1 2 3 
sigma=10
speed=50
c style ?:0
f style ?:0
x style ?:0
arg of beta is:0
beta did not associate with a variable, so you need to parse it by youself
the input sequene:
0 1 2 3 
remain argc=5
remain argv:
 remain
 arguments
 1
 2
 3
```
 
```
% ./a.out -xf --sigma -50 
option --sigma argument illegal: must greater then zero
```

```
% ./a.out --sequence=4,3,2,1 
sigma=10
speed=0.5
c style ?:0
f style ?:0
x style ?:0
arg of beta is:0
beta did not associate with a variable, so you need to parse it by youself
the input sequene:
4 3 2 1 
remain argc=0
remain argv:
```

```
% ./a.out --sequence=4,3,2,1,0 
option --sequence argument illegal: too much int
```
上述代码初`add`始化了一个用程序名命名的解析器，拥有一个参数sigma，短参数为s，拥有一个验证器,验证sigma必须大于0，sigma默认值为10
第二个`add`定义了一个speed选项,没有短选项,默认值为0.5,没有验证器,第三个到第五个`add`只定义了3个短选项,可以通过`opt_has`查找是否包含选项

`help`定义了一个help，当解析命令行参数的时候出现--help或者-h,直接输出帮助文档输出格式将仿照boost program options

函数`more_help`定义为注册一个自定义的可以带参数的帮助现显示命令，以支持显示子命令帮助，用户需要提供一个打印函数，可选的上下文，这里将注册的`sub_cmd`字符串作为上下文,以上几行c代码相当于下面的c++ program options代码。

`group` 可以添加用于显示的分组，`text`可以添加独立的文字描述。

```cpp
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
        ("sigma,s", po::value(&sigma)->required()->default_value(10), "specify the sigma")
        ("help,h", "display help")
        (",c", "toggle c style")
        (",f", "toggle f style")
        (",x", "toggle x style")
        ("command", po::value(&sub_cmd), "show help of sub commands");

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
       cout << sub_cmd << " is a sub cmd " << endl;
      return 0; 
    }

    if ( sigma < 0 )
    {
        cerr << "sigma must be greater than zero " << endl;
        return 1;
    }
    cout << sigma << endl;
    return 0;
}
```

本库被设计为可以扩展，目前暂时提供了int,double,c 风格字符串，int数组,通过提供适当的`OptionValue`可以支持更多的格式化数据，诸如直接将参数中的字符串格式化填写到`struct sockaddr`, glib的`GList`, 以及其他常用clib的数据结构。所有`OptionValue`将使用类似`opt_int, opt_double, opt_string`的链式构建器来创建.


###基本API
#### opt\_init(char* description) 初始化构造器
- `->group(char* name)` 开始一个组
- `->add(char const* option_name, char const* help, OptionValue* argument_value)` 第三个参数可以为NULL,表示用户自己解析, option\_name如果只有一个字符，作为短参数，option\_name使用,分割长参数与短参数,如果只有短参数,那个短参数作为实际option\_name,否则长参数作为option\_name
- `->help(char const* help)` 生成帮助选项--help, -h
- `->more_help(char const* option_name, char const* help, OptionValue* value_for_more_help, void (*printer)(void*), void* printer_context)`  添加一个查询子命令详细帮助的选项
- `->parse_into_(int argc, char const* const* argv, OptionParser* pparser)`解析命令行，结果存入pparser,如果存在不符合要求的选项,不符合选项要求的参数,将自动报错退出,如果出现help与more help参数,则现实help与调用printer

#### OptionParser
- `int opt_has(OptionParser parser, char const* option_name)` 查询是否出现对应选项(选项如果是_required_则必定返回1)
- `char const* opt_get_arg(OptionParser parser, char const* option_name)`查询对应选项的参数,如果选项存在但没有参数并且有默认值,则返回默认值对应的字符串
- `opt_print(OptionParser parser), opt_fprint(FILE*, OptionParser parser)`打印完成的帮助文档
- `opt_free(OptionParser parser)`释放内存


#### OptionValue
```c
typedef struct option_value* OptionValue;
typedef struct option_value_ops* OptionValueOps;

struct option_value
{
    void* pointer;//输出变量地址，NULL不输出
    int required;//是否必须存在
    int has_default;//是否有默认值
    OptionValueOps ops;//操作
};
struct option_value_ops
{
    char const* (*validate)(OptionValue self, char const* arg);//验证arg的正确性, NULL默认正确，函数返回NULL表示正确，否则返回错误信息
    char const* (*parse)(OptionValue self, char const* arg);//解析arg,将结果放入到self->pointer,成功返回NULL,否则返回错误信息
    char const* (*display_default)(OptionValue self);//返回默认值的字符串
    void (*store_default)(OptionValue self);//将默认值放入到self->pointer中
    void (*free)(OptionValue self);//释放数据结构
};
```

####OptionValueBuilder
- `opt_int(int*)`, `opt_double(double*)`, `opt_string(char const**)` 生成一个对应类型的构造器，如果提供参数，在解析命令行后会将对应的值传输到对应变量中,返回一个构造器，构造器的地址兼容OptionValue\*
    - `->required()`标记必须需要的选项
    - `->validator( char const* (*user_validator)(type, void*) )`注册一个用户定义的验证器，type可能为int，double,字符串没有验证器
    - `->context(void*)` 添加上下文,会被传入到用户验证器中
    - `->free(void (*user_free_context)(void*))`注册一个用户自定义上下文释放函数,如果注册,parser释放时会被调用
    - `->base(int)` int拥有指定base的函数
    - `->default_value(type)` 设置默认值

- `opt_ints(int *, int* size)`,`opt_doubles(double*, int* size)`,`opt_strings(char const** , int* size)` 一个数组构造器, size必须提供,size指针的值表示数组最大值,解析后\*size会设置为真正的大小
    - `->required()`
    - `->validator( char const* (*user_validator(type* array, int size, void*) ))`
    - `->context(void*)`
    - `->free(void (*)(void*))`
    - `->default_value( type *, int )` 设置默认数组
    - `->default_value(char const* arg, ...)` strings 使用变长参数设置默认数组, 必须NULL结尾
    - `delimiters(char const*)` 分隔符,默认","
opt\_strings的输出数组中字符串的生命周期与parser相同,parser释放后,访问字符串数组的行为未定义
