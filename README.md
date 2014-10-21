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
    opt_init(argv[0])
        ->add("sigma,s", "specify the sigma", opt_int(&sigma)->required()->validator(gerater_then_zero)->default_value(10))
        ->add("speed", "specify the speed", opt_double(&speed)->required()->default_value(.5))
        ->add("c", "toggle c style", NULL)
        ->add("f", "toggle f style", NULL)
        ->add("x", "toogle x style", NULL)
        ->help("show help")
        ->more_help("command", "print help of sub commands", opt_string(&sub_cmd), print_cmd, &sub_cmd)->
    parse_into(argc, argv, &parser);

    printf("sigma=%d\n",sigma);
    printf("speed=%g\n", speed);
    printf("remain argc=%d\n", parser->argc);
    printf("remain argv:\n");
    printf("c style ?:%d\n", opt_has(parser, "c"));
    printf("f style ?:%d\n", opt_has(parser, "f"));
    printf("x style ?:%d\n", opt_has(parser, "x"));
    for(int i = 0; i < parser->argc; ++i)
        printf(" %s\n", parser->argv[i]);

    opt_free(parser);
    return 0;
}
```
####输出样例:
```bash
% ./a.out -xf 
sigma=10
speed=0.5
remain argc=0
remain argv:
c style ?:0
f style ?:1
x style ?:1


% ./a.out --speed=50.0 remain arguments 1 2 3
sigma=10
speed=50
remain argc=5
remain argv:
c style ?:0
f style ?:0
x style ?:0
 remain
 arguments
 1
 2
 3

% ./a.out -xf --sigma -50 
option --sigma argument illegal: must greater then zero

% ./a.out 
./a.out:
  -s [ --sigma ] arg (=10)                specify the sigma
  -v [ --speed ] arg (=0.5)               specify the speed
  -c                                      toggle c style
  -f                                      toggle f style
  -x                                      toogle x style
  -h [ --help ]                           show help
  --command arg                           print help of sub commands
```
上述代码初`add`始化了一个用程序名命名的解析器，拥有一个参数sigma，短参数为s，拥有一个验证器,验证sigma必须大于0，sigma默认值为10
第二个`add`定义了一个speed选项,没有短选项,默认值为0.5,没有验证器,第三个到第五个`add`只定义了3个短选项,可以通过`opt_has`查找是否包含选项

`help`定义了一个help，当解析命令行参数的时候出现--help或者-h,直接输出帮助文档输出格式将仿照boost program options

函数`more_help`定义为注册一个自定义的可以带参数的帮助现显示命令，以支持显示子命令帮助，用户需要提供一个打印函数，可选的上下文，这里将注册的`sub_cmd`字符串作为上下文,以上几行c代码相当于下面的c++ program options代码。

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

-本库还在开发中-

-文档将逐步完善-
