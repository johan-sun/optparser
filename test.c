#include  <stdio.h>
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
    OptionParser parser;
    int sigma;
    double speed;
    char const* sub_cmd = NULL;
    opt_init(argv[0])
        ->add("sigma,s", "special the sigma", opt_int(&sigma)->required()->validator(gerater_then_zero)->default_value(10))
        ->add("speed,v", "special the speed", opt_double(&speed)->required()->default_value(.5))
        ->add("c", "toggle c style", NULL)
        ->add("f", "toggle f style", NULL)
        ->add("x", "toogle x style", NULL)
        ->help("show help")
        ->more_help("command", "print help of sub command", opt_string(&sub_cmd), print_cmd, &sub_cmd)->
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
