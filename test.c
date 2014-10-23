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
