#include    "optparser.h"
#include  <stdlib.h>
#include  <string.h>
#include  <ctype.h>
#include  <assert.h>


struct option
{
    char const* name;
    char short_name;
    int exist;
    char const* possible_arg;
    char const* help;
    char* buff;
    void (*help_printer)(void*);
    void* context;
    OptionValue value;
    struct option* next;
    struct option* prev;
};

OptionCmdChain option_add(char const* option_name, char const* help, void* option_value_builder);
OptionCmdChain option_more_help(char const* option_name, char const* help, void* option_value_builder, void(*printer)(void*), void* context);
OptionCmdChain option_help(char const* help);
void option_parse_into(int argc, char const * const*argv, OptionParser* parser);
static struct option* g_options = NULL;
struct option_cmd_chain g_cmd_chain = {
    .add = option_add,
    .parse_into = option_parse_into,
    .more_help = &option_more_help,
    .help = &option_help,
};
OptionCmdChain opt_init(char const* desc)
{
    g_options = (struct option*)malloc(sizeof *g_options);
    bzero(g_options, sizeof *g_options);
    g_options->next = g_options->prev = g_options;
    g_options->name = desc;
    return &g_cmd_chain;
}

OptionCmdChain option_more_help(char const* option_name, char const* help, void* option_value_builder, void(*printer)(void*), void* context)
{
    option_add(option_name, help, option_value_builder);
    struct option* o = g_options->prev;
    o->help_printer = printer;
    o->context = context;
    return &g_cmd_chain;
}
OptionCmdChain option_help(char const* help)
{
    return option_add("help,h", help, NULL);
}
OptionCmdChain option_add(char const* option_name, char const* help, void* option_value_builder)
{
    assert(option_name);
    struct option * opt = (struct option*)malloc(sizeof *opt);
    bzero(opt, sizeof *opt);
    if(strlen(option_name) == 1)
    {
        opt->name = option_name;
        opt->short_name = *option_name;
    }else
    {
        opt->buff = strdup(option_name);
        char *p = strchr(opt->buff, ',');
        if ( p )
        {
            *p = '\0';
            opt->short_name = *(p+1);
        }
        opt->name = opt->buff;
    }
    for(struct option* p = g_options->next; p != g_options; p = p->next)
    {
        if ( (opt->short_name == p->short_name && opt->short_name != '\0')|| strcmp(opt->name, p->name) == 0 )
        {
            fprintf(stderr, "redefine option name or short name\n");
            exit(1);
        }
    }
    opt->help = help;
    if ( option_value_builder )
    {
        OptionValue ov = *(OptionValue*)(option_value_builder);
        opt->value = ov;
    }
    opt->prev = g_options->prev;
    opt->next = g_options;
    g_options->prev->next = opt;
    g_options->prev = opt;
    return &g_cmd_chain;
}


static struct option* find_option(char sh)
{
    for(struct option* p = g_options->next; p != g_options; p = p->next)
    {
        if ( sh == p->short_name )
            return p;
    }
    return NULL;
}
static struct option* find_option_long(char const* name)
{
    for(struct option* p = g_options->next; p != g_options; p = p->next)
    {
        if ( strcmp(name, p->name)  == 0)
            return p;
    }
    return NULL;

}
void check_option_repeat_or_no_def(struct option* o, char const* arg)
{
    if ( o == NULL)
    {
        fprintf(stderr, "option %s not defined\n", arg);
        exit(1);
    }
    if ( o->exist )
    {
        fprintf(stderr, "Waring:option %s repeat\n", o->name);
    }
}
#define PRE(s) (strlen(s) == 1?"-":"--")

static int may_be_arg(const char* arg)
{
    if ( *arg != '-' )
        return 1;
    if (strlen(arg) > 2 && *arg == '-' && (isdigit(arg[1]) || arg[1] == '.'))
        return 1;
    return 0;
}

#define foreach_option( var, head ) for(struct option* var = ((struct option*)(head))->next; var != head; var = var->next)
static int parse_cmd(int argc, char const * const* argv)
{
    int i = 1;
    while(i < argc)
    {
        if (may_be_arg(argv[i]))
            break;
        if ( strlen(argv[i]) == 2 && *argv[i] == '-' && *(argv[i]+1) != '-')
        {
            char short_name = *(argv[i]+1);
            struct option* o = find_option(short_name);
            check_option_repeat_or_no_def(o, argv[i]);
            o->exist = 1;
            if (o->value && i+1 < argc && may_be_arg(argv[i+1]) )
            {
                o->possible_arg = argv[i+1];
                ++i;
            }
            ++i;
        }else if ( strlen(argv[i]) > 2 && *argv[i] == '-' && *(argv[i]+1) != '-')
        {
            char const* last = argv[i] + strlen(argv[i]);
            struct option* o = NULL;
            for(char const* p = argv[i]+1; p < last; ++p)
            {
                o = find_option(*p);
                check_option_repeat_or_no_def(o, argv[i]);
                o->exist = 1;
                if ( o->value )
                {
                    if ( p == last - 1 )
                    {
                        if (i + 1< argc && may_be_arg(argv[i+1]))
                        {
                            o->possible_arg = argv[i+1];
                            ++i;
                        }
                    }else
                    {
                        o->possible_arg = p+1;
                    }
                    break;
                }
            }
            ++i;
        }else if ( strlen(argv[i]) > 2 && argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] != '-')
        {
            char * name = strdup(argv[i] + 2);
            char * p = strchr(name, '=');
            char const* possible_arg = NULL;
            if ( p  ) 
            {
                *p = '\0';
                possible_arg = argv[i] + 2 + (p - name) + 1;
            }
            struct option * o = find_option_long(name);
            free(name);
            check_option_repeat_or_no_def(o , argv[i]);
            o->exist = 1;
            if ( possible_arg && *possible_arg != '\0' )
            {
                o->possible_arg = possible_arg;
            }else if ( i +1 < argc && may_be_arg(argv[i+1]) )
            {
                o->possible_arg = argv[i+1];
                ++i;
            }
            ++i;
        }else{
            fprintf(stderr, "illegal format option %s\n", argv[i]);
            exit(1);
        }
    }
    return i;
}
static void notify_output(int for_helper)
{
    foreach_option(o, g_options)
    {
        if ( for_helper && !o->help_printer)  continue;
        if ( !for_helper && o->help_printer ) continue;
        if ( ! o->value ) continue;
        if ( o->exist && o->possible_arg )
        {
            if(o->value->ops->validate)
            {
                char const* err_msg = o->value->ops->validate(o->value, o->possible_arg);
                if ( err_msg )
                {
                    fprintf(stderr, "option %s%s argument illegal: %s\n", PRE(o->name),o->name, err_msg);
                    exit(1);
                }
            }
            if ( o->value->pointer )
            { 
                char const* err_msg = o->value->ops->parse(o->value, o->possible_arg);
                if ( err_msg )
                {
                    fprintf(stderr, "option %s%s argument illegal: %s\n", PRE(o->name),o->name, err_msg);
                    exit(1);
                }
                continue;
            }        
        }

        if ( o->value->required )
        {
            if ( !o->exist && !o->value->has_default)
            {
                fprintf(stderr, "option %s%s required but missing\n", PRE(o->name),o->name);
                exit(1);
            }
            if ( o->exist && o->value->pointer && o->value->has_default)
            {
                o->value->ops->store_default(o->value);
                continue;
            }
            if (o->exist && !o->value->has_default)
            {
                fprintf(stderr, "option %s%s need argument but missing\n", PRE(o->name),o->name);
                exit(1);
            }
        }else if ( o->exist )
        {
            if ( !o->value->has_default )
            {
                fprintf(stderr, "option %s%s need argument but missing\n", PRE(o->name),o->name);
                exit(1);
            }else if (o->value->pointer)
            {
                o->value->ops->store_default(o->value);
            }
            
        }
    }
}
void option_parse_into(int argc, char const * const* argv, OptionParser* pparser)
{
    *pparser = (struct optparser*)malloc(sizeof(**pparser));
    (*pparser)->_private = g_options;

    int i = parse_cmd(argc, argv);

    if ( i == 1  && g_options->next != g_options)
    {
        opt_print(*pparser);
        exit(0);
    }

    foreach_option(o , g_options)
    {
        if ( strcmp("help", o->name) == 0 && o->exist)
        {
            opt_print(*pparser); 
            exit(0);
        }
    }

    const int for_helper = 1;
    notify_output(for_helper);
    foreach_option(o , g_options)
    {
        if( o->exist && o->help_printer )
        {
            o->help_printer(o->context);
            exit(0);
        }
    }
    notify_output(!for_helper);
    
    (*pparser)->argc = argc - i;
    (*pparser)->argv = argv + i;
    g_options = NULL;
}


int opt_has(OptionParser parser, char const* key)
{
    foreach_option(o, parser->_private)
    {
        if ( strcmp(key, o->name) == 0  )
        {
            if ( o->exist || (o->value && o->value->has_default) )
                return 1;
            break;
        }
    }
    return 0;
}
char const* opt_get_arg(OptionParser parser, char const* key)
{
    foreach_option(o, parser->_private)
    {
        if ( strcmp(key, o->name)  == 0 )
        {
            if ( o->exist )
            {
                if (o->possible_arg)
                    return o->possible_arg;
                if (o->value && o->value->has_default)
                    return o->value->ops->display_default(o->value);
            }
            break;
        }
    }
    return NULL;
}
void opt_fprint(FILE* file, OptionParser parser)
{

}
void opt_print(OptionParser parser)
{
    opt_fprint(stdout, parser);
}
static void free_option(struct option* o)
{
    if(o->buff)
        free(o->buff);
    if(o->value)
        o->value->ops->free(o->value);
    free(o);
}
void opt_free(OptionParser parser)
{
    struct option * p = (struct option*)parser->_private;
    p = p->next;
    while( p != parser->_private )
    {
        struct option* free_p = p;
        p = p->next;
        free_option(free_p);
    }
    free_option((struct option*)parser->_private);
    free(parser); 
}
