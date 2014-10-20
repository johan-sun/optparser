#include    "optparser.h"

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
int main(int argc, char *argv[])
{
    /*
    int n;
    OptionValue v = opt_int(&n)->required()->validator(gerater_then_zero)->default_value(10)->base(0)->_data;
    printf("required:%d\n", v->required);
    v->ops->parse(v, "123");
    printf("%d\n", n);
    v->ops->store_default(v);
    printf("%d\n", n);
    char const* err = v->ops->validate(v, "-123");
    if ( err )
        printf("err:%s\n", err);
    err = v->ops->validate(v, "a123");
    if ( err )
        printf("err:%s\n", err);

    v->ops->free(v);*/
    OptionParser parser;
    int sigma;
    char const* sub_cmd;
    opt_init(argv[0])
        ->add("sigma,s", "special the sigma", opt_int(&sigma)->required()->validator(gerater_then_zero)->default_value(10))
        ->help("show help")
        ->more_help("command", "print help of sub command", opt_string(&sub_cmd), print_cmd, &sub_cmd)
        ->parse_into(argc, argv, &parser);
    printf("%d\n",sigma);
    opt_free(parser);
    return 0;
}
