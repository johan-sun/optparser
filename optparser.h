#ifndef  OPTPARSER_H
#define  OPTPARSER_H

#include  <stdio.h>
#ifdef __cplusplus
extern "C"{
#endif

typedef struct option_value* OptionValue;
typedef struct option_value_ops* OptionValueOps;

struct option_value
{
    void* pointer;
    int required;
    int has_default;
    OptionValueOps ops;
};
struct option_value_ops
{
    char const* (*validate)(OptionValue self, char const* arg);
    char const* (*parse)(OptionValue self, char const* arg);
    char const* (*display_default)(OptionValue self);
    void (*store_default)(OptionValue self);
    void (*free)(OptionValue self);
};

typedef struct optparser
{
    int argc;
    char const * const* argv;
    void* _private;
}* OptionParser;

typedef struct option_cmd_chain* OptionCmdChain;
struct option_cmd_chain
{
    //actually function is (char const* option_name, char const* help, OptionValue* pvalue)
    //after add , *pvalue will be set to NULL, mean the value has move into optparser
    //the optparser will manage is life time, use void* is to support the optvalue builder
    //the builder's first member is OptionValue, so the address of builder will equal to
    //the address of OptionValue member, it is OptionValue*
    OptionCmdChain (*add)(char const* option_name, char const* help, void* pvalue);
    OptionCmdChain (*text)(char const* txt);
    OptionCmdChain (*group)(char const* group_name);
    OptionCmdChain (*more_help)(char const* help_name, char const* help, void* pvalue, void(*printer)(void*), void* context);
    OptionCmdChain (*help)(char const* help);
    void (*parse_into)(int argc, char const * const* argv, OptionParser* pparser);
};
OptionCmdChain opt_init(char const* desc);
int opt_has(OptionParser parser, char const* key);
char const* opt_get_arg(OptionParser parser, char const* key);
void opt_fprint(FILE*, OptionParser parser);
void opt_print(OptionParser parser);
void opt_free(OptionParser);

// option value builders {{{
typedef struct opt_value_user_builder_interface* OptValueUserBuilderInterface;
struct opt_value_user_builder_interface
{
    OptionValue _data;
    OptValueUserBuilderInterface (*required)();
    OptValueUserBuilderInterface (*context)(void* ctx);
    OptValueUserBuilderInterface (*validator)( char const* (*user_validate)(char const* arg, void* ctx) );
    OptValueUserBuilderInterface (*parser)(char const* (*user_parse)(char const* arg, void* out, void* ctxj));
    OptValueUserBuilderInterface (*default_store)(void (*copy)(void* default_value, void* out));
    OptValueUserBuilderInterface (*default_value)(void*);
    OptValueUserBuilderInterface (*default_display)(char const*);
    OptValueUserBuilderInterface (*free)(void (*ctx_free)(void*));
};
OptValueUserBuilderInterface opt_arg(void* pointer);
//======================================================
typedef struct opt_int_builder_interface* OptIntBuilderInterface;
struct opt_int_builder_interface
{
    OptionValue _data;
    OptIntBuilderInterface (*required)();
    OptIntBuilderInterface (*validator)(char const* (*)(int,void*));
    OptIntBuilderInterface (*default_value)(int);
    OptIntBuilderInterface (*base)(int);
    OptIntBuilderInterface (*context)(void*);
    OptIntBuilderInterface (*free)(void (*ctx)(void*));
};
OptIntBuilderInterface opt_int(int* pointer);
//======================================================
typedef struct opt_double_builder_interface* OptDoubleBuilderInterface;
struct opt_double_builder_interface
{
    OptionValue _data;
    OptDoubleBuilderInterface (*required)();
    OptDoubleBuilderInterface (*validator)(char const*(*)(double d, void*));
    OptDoubleBuilderInterface (*default_value)(double);
    OptDoubleBuilderInterface (*context)(void*);
    OptDoubleBuilderInterface (*free)(void(*)(void*));
};
OptDoubleBuilderInterface opt_double(double* pointer);
//=====================================================
typedef struct opt_string_builder_interface* OptStringBuilderInterface;
struct opt_string_builder_interface
{
    OptionValue _data;
    OptStringBuilderInterface (*required)();
    OptStringBuilderInterface (*default_value)(char const*);
};
OptStringBuilderInterface opt_string(char const** p);
//=====================================================
typedef struct opt_ints_builder_interface* OptIntsBuilderInterface;
struct opt_ints_builder_interface
{
    OptionValue _data;
    OptIntsBuilderInterface (*required)();
    OptIntsBuilderInterface (*default_value)(int const*, int);
    OptIntsBuilderInterface (*delimiters)(char const*);
    OptIntsBuilderInterface (*validator)(char const* (*)(int const*, int, void*));
    OptIntsBuilderInterface (*context)(void*);
    OptIntsBuilderInterface (*free)(void(*)(void*));
};
OptIntsBuilderInterface opt_ints(int* pointer, int* n);
//=====================================================
// }}}
#ifdef __cplusplus
}
#endif

#endif  /*OPTPARSER_H*/
