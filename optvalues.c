#include    "optparser.h"
#include  <stdlib.h>
#include  <string.h>
#include  <assert.h>
#include  <stdarg.h>
#define container(pointer, ptype, field) ((ptype)((char*)(pointer) -  (char*)&((ptype)0)->field))
#define cast(type, self) container(self, type, _base)


// option value user {{{ 
struct opt_value_user
{
    struct option_value _base;
    void* context;
    void* default_value;
    void (*default_store)(void*, void*);
    char const* default_display;
    char const* (*user_validator)(char const*, void*);
    char const* (*user_parser)(char const*, void*, void*);
    void (*context_free)(void*);
};
#define value_user_cast(self) cast(struct opt_value_user*, self)
static char const* value_user_validate(OptionValue self, char const* arg)
{
    struct opt_value_user* u_self = value_user_cast(self);
    if (u_self->user_validator )
        return u_self->user_validator(arg, u_self->context);
    return NULL;
}
static char const* value_user_parse(OptionValue self, char const* arg)
{
    assert( self->pointer );
    struct opt_value_user* u_self = value_user_cast(self);
    if ( u_self->user_parser )
        return u_self->user_parser(arg, self->pointer, u_self->context);
    return NULL;
}
static void value_user_free(OptionValue self)
{
    struct opt_value_user* u_self = value_user_cast(self);
    if ( u_self->context_free && u_self->context )
        u_self->context_free(u_self->context); 
    free(u_self);
}
static void value_user_store_default(OptionValue self)
{
    struct opt_value_user* u_self = value_user_cast(self);
    if ( self->has_default && u_self->default_store )
        u_self->default_store(u_self->default_value, self->pointer);
}
static char const* value_user_display_default(OptionValue self)
{
    char const* s = value_user_cast(self)->default_display;
    if ( s == NULL )
        return "object";
    return s;
}
struct option_value_ops g_value_user_ops = {
    .validate = &value_user_validate,
    .parse = &value_user_parse,
    .store_default = &value_user_store_default,
    .display_default= &value_user_display_default,
    .free = value_user_free
};

static OptValueUserBuilderInterface ubi_set_required();
static OptValueUserBuilderInterface ubi_set_context(void* context);
static OptValueUserBuilderInterface ubi_set_validator(char const* (*user_validator)(char const*, void*));
static OptValueUserBuilderInterface ubi_set_parser(char const* (*user_parser)(char const*, void*, void*));
static OptValueUserBuilderInterface ubi_set_default_value(void* value);
static OptValueUserBuilderInterface ubi_set_free(void (*on_free)(void*));
static OptValueUserBuilderInterface ubi_set_default_store(void (*store)(void*, void*));
static OptValueUserBuilderInterface ubi_set_default_display(char const*);

struct opt_value_user_builder_interface g_user_builder = {
    ._data = NULL,
    .required = &ubi_set_required,
    .validator = &ubi_set_validator,
    .parser = &ubi_set_parser,
    .default_value = &ubi_set_default_value,
    .free = &ubi_set_free,
    .context = &ubi_set_context,
    .default_store = &ubi_set_default_store,
    .default_display = & ubi_set_default_display,
};

OptValueUserBuilderInterface opt_arg(void* pointer)
{
   struct opt_value_user* value = (struct opt_value_user*)malloc(sizeof(*value));
   bzero(value, sizeof *value);
   value->_base.pointer = pointer;
   value->_base.ops = &g_value_user_ops;
   g_user_builder._data = &value->_base;
   return &g_user_builder;
}
#define this_user_value() container(g_user_builder._data, struct opt_value_user*, _base)
static OptValueUserBuilderInterface ubi_set_required()
{
    g_user_builder._data->required = 1;
    return &g_user_builder;
}
static OptValueUserBuilderInterface ubi_set_validator(char const* (*user_validator)(char const*, void*))
{
    this_user_value()->user_validator = user_validator;
    return &g_user_builder;
}
static OptValueUserBuilderInterface ubi_set_parser(char const* (*user_parser)(char const*, void*, void*))
{
    this_user_value()->user_parser = user_parser;
    return &g_user_builder;
}
static OptValueUserBuilderInterface ubi_set_default_value(void* value)
{
    this_user_value()->default_value = value;
    this_user_value()->_base.has_default = 1;
    return &g_user_builder;
}

static OptValueUserBuilderInterface ubi_set_default_display(char const* s)
{
    this_user_value()->default_display = s;
    return &g_user_builder;
}
static OptValueUserBuilderInterface ubi_set_free(void (*on_free)(void*))
{
    this_user_value()->context_free = on_free;
    return &g_user_builder;
}

static OptValueUserBuilderInterface ubi_set_context(void* context)
{
    this_user_value()->context = context;
    return &g_user_builder;
}

static OptValueUserBuilderInterface ubi_set_default_store(void (*store)(void*, void*))
{
    this_user_value()->default_store = store;
    return &g_user_builder;
}
// }}}
// option value int{{{  
struct opt_value_int
{
    struct option_value _base;
    int default_int;
    int base;
    void* context;
    char const* (*int_validator)(int, void*);
    void (*context_free)(void*);
};

#define value_int_cast(self) cast(struct opt_value_int*, self)
static char const* value_int_validate(OptionValue self, char const* arg)
{
    char* endptr;
    struct opt_value_int* i_self = value_int_cast(self);
    int n = strtol(arg, &endptr, i_self->base);
    if ( *endptr == '\0')
    {
        if( i_self->int_validator ) return i_self->int_validator(n, i_self->context);
        return NULL;
    }
    return "not a int";
}

static char const* value_int_parse(OptionValue self, char const* arg)
{
    assert( self->pointer );
    int n = strtol(arg, NULL, value_int_cast(self)->base);
    *(int*)(self->pointer) = n;
    return NULL;
}

static void value_int_free(OptionValue self)
{
    struct opt_value_int* int_self = value_int_cast(self);
    if ( int_self->context_free && int_self->context)
        int_self->context_free(int_self->context);
    free(int_self);
}

static void value_int_store_default(OptionValue self)
{
    assert(self->pointer);
    *(int*)self->pointer = value_int_cast(self)->default_int;
}
char const* value_int_display_default(OptionValue self)
{
    static char str[32];
    snprintf(str, sizeof str, "%d",value_int_cast(self)->default_int);
    return str;
}
struct option_value_ops g_value_int_ops = {
    .validate = &value_int_validate,
    .parse = &value_int_parse,
    .store_default = &value_int_store_default,
    .display_default = &value_int_display_default,
    .free = &value_int_free
};
static OptIntBuilderInterface int_set_required();
static OptIntBuilderInterface int_set_validator(char const* (*user_validator)(int n, void*));
static OptIntBuilderInterface int_set_default_value(int def);
static OptIntBuilderInterface int_set_base(int b);
static OptIntBuilderInterface int_set_context(void* context);
static OptIntBuilderInterface int_set_context_free(void (*)(void*));
struct opt_int_builder_interface g_int_builder = {
    ._data = NULL,
    .required = &int_set_required,
    .validator = &int_set_validator,
    .default_value = &int_set_default_value,
    .base = &int_set_base,
    .context = &int_set_context,
    .free = &int_set_context_free,
};
#define this_int_value() value_int_cast(g_int_builder._data)
OptIntBuilderInterface opt_int(int* p)
{
    struct opt_value_int* value = (struct opt_value_int*)malloc(sizeof(*value));
    bzero(value, sizeof(*value));
    value->base = 10;
    value->_base.ops = &g_value_int_ops;
    value->_base.pointer = p;
    g_int_builder._data = &value->_base;
    return &g_int_builder;
}
static OptIntBuilderInterface int_set_required()
{
    this_int_value()->_base.required = 1;
    return &g_int_builder;
}

static OptIntBuilderInterface int_set_validator(char const* (*user_validator)(int, void*))
{    
    this_int_value()->int_validator = user_validator;
    return &g_int_builder;
}
static OptIntBuilderInterface int_set_default_value(int n)
{
    this_int_value()->default_int = n;
    this_int_value()->_base.has_default = 1;
    return &g_int_builder;
}
static OptIntBuilderInterface int_set_base(int b)
{
    this_int_value()->base = b;
    return &g_int_builder;
}
static OptIntBuilderInterface int_set_context(void* context)
{
    this_int_value()->context = context;
    return &g_int_builder;
}
static OptIntBuilderInterface int_set_context_free(void (*free)(void*))
{
    this_int_value()->context_free = free;
    return &g_int_builder;
}
// }}}
// option value double {{{
struct opt_value_double
{
    struct option_value _base;
    double default_double;
    void* context;
    char const* (*validator)(double, void*);
    void (*context_free)(void*);
};


#define value_double_cast(self) cast(struct opt_value_double*, self)
static char const* value_double_validate(OptionValue self, char const* arg)
{
    char* endptr;
    double d = strtod(arg, &endptr);
    struct opt_value_double* d_self = value_double_cast(self);
    if ( *endptr == '\0')
    {
         if (d_self->validator)  return d_self->validator(d, d_self->context);
         return NULL;
    }
    return "not a double";
}

static char const* value_double_parse(OptionValue self, char const* arg)
{
    if ( self->pointer )
    {
        double d = strtod(arg, NULL);
        *(double*)self->pointer = d;
    }
    return NULL;
}

static void value_double_store_default(OptionValue self)
{
    struct opt_value_double* d_self = value_double_cast(self);
    *(double*)self->pointer = d_self->default_double;
}

static char const* value_double_display_default(OptionValue self)
{
    static char str[32];
    snprintf(str, sizeof str, "%g", value_double_cast(self)->default_double);
    return str;
}
static void value_double_free(OptionValue self)
{
    struct opt_value_double* d_self = value_double_cast(self);
    if ( d_self->context_free && d_self->context )
        d_self->context_free(d_self->context);
    free(d_self);
}
struct option_value_ops g_value_double_ops = {
    .validate = &value_double_validate,
    .parse = &value_double_parse,
    .store_default = &value_double_store_default,
    .display_default = &value_double_display_default,
    .free = &value_double_free,
};

static OptDoubleBuilderInterface double_set_required();
static OptDoubleBuilderInterface double_set_validator(char const* (*user_validator)(double, void*));
static OptDoubleBuilderInterface double_set_default_value(double d);
static OptDoubleBuilderInterface double_set_context(void*);
static OptDoubleBuilderInterface double_set_context_free(void(*free)(void*));
struct opt_double_builder_interface g_double_builder = {
    ._data = NULL,
    .required = &double_set_required,
    .validator = &double_set_validator,
    .default_value = &double_set_default_value,
    .context = &double_set_context,
    .free = &double_set_context_free,
};
#define this_double_value() value_double_cast(g_double_builder._data)
OptDoubleBuilderInterface opt_double(double* pointer)
{
    struct opt_value_double* val = (struct opt_value_double*)malloc(sizeof(*val));
    bzero(val, sizeof(*val));
    val->_base.pointer = pointer;
    val->_base.ops = &g_value_double_ops;
    g_double_builder._data = & val->_base;
    return &g_double_builder;
}

static OptDoubleBuilderInterface double_set_required()
{
    this_double_value()->_base.required = 1;
    return &g_double_builder;
}

static OptDoubleBuilderInterface double_set_validator(char const* (*user_validator)(double, void*))
{
    this_double_value()->validator = user_validator;
    return &g_double_builder;
}

static OptDoubleBuilderInterface double_set_default_value(double d)
{
    this_double_value()->default_double = d;
    this_double_value()->_base.has_default = 1;
    return &g_double_builder;
}

static OptDoubleBuilderInterface double_set_context(void* context)
{
    this_double_value()->context = context;
    return &g_double_builder;
}

static OptDoubleBuilderInterface double_set_context_free(void(*free)(void*))
{
    this_double_value()->context_free = free;
    return &g_double_builder;
}
//}}}
// option value string {{{
struct opt_value_string
{
    struct option_value _base;
    char const* default_string;
};
#define value_string_cast(self) cast(struct opt_value_string*, self)
static char const* value_string_parse( OptionValue self, char const* args )
{
    assert( self->pointer );
    *(char const **)self->pointer = args;
    return NULL;
}

static void value_string_store_default( OptionValue self )
{
    struct opt_value_string* s_self = value_string_cast(self);
    assert( self->pointer );
    *(char const**)self->pointer = s_self->default_string;
}
static void value_string_free(OptionValue self)
{
    free( value_string_cast(self) );
}
static char const* value_string_display_default(OptionValue self)
{
    return value_string_cast(self)->default_string;
}
struct option_value_ops g_value_string_ops = {
    .validate = NULL,
    .parse = &value_string_parse,
    .store_default = &value_string_store_default,
    .display_default = &value_string_display_default,
    .free = &value_string_free
};

static OptStringBuilderInterface string_required();
static OptStringBuilderInterface string_default_value(char const*);
struct opt_string_builder_interface g_string_builder = {
    ._data = NULL,
    .required = &string_required,
    .default_value = &string_default_value,
};
#define this_string_value() value_string_cast(g_string_builder._data)
OptStringBuilderInterface opt_string(char const** p)
{
    struct opt_value_string* svalue = (struct opt_value_string*)malloc(sizeof * svalue);
    bzero(svalue, sizeof*svalue);
    svalue->_base.pointer = p;
    svalue->_base.ops = & g_value_string_ops;
    g_string_builder._data = & svalue->_base;
    return &g_string_builder;
}
static OptStringBuilderInterface string_required()
{
    this_string_value()->_base.required = 1;
    return &g_string_builder;
}
static OptStringBuilderInterface string_default_value(char const* p)
{
    this_string_value()->default_string = p;
    return &g_string_builder;
}
///}}}
// option value ints {{{
struct opt_value_ints
{
    struct option_value _base;
    int max_output_n;
    int* output_n;
    int base;
    int const* default_ints;
    int default_n;
    char const* dlms;
    char const* (*validator)(int const*, int, void*);
    void* context;
    void (*context_free)(void*);
};
#define value_ints_cast(self) cast(struct opt_value_ints*, self)
char const* value_ints_validate(OptionValue self, char const* arg)
{
    struct opt_value_ints* is_self = value_ints_cast(self);
    int * arr = (int*)malloc(sizeof(int)*is_self->max_output_n);
    char* buf = strdup(arg);
    static char err_buf[100];
    int i;
    char* p;
    char const* err_msg = NULL;
    for(p = strtok(buf, is_self->dlms),i = 0; p; p = strtok(NULL, is_self->dlms), ++i)
    {
        if ( i ==  is_self->max_output_n)
        {
            err_msg = "too much int";
            break;
        }
        char * endptr;
        int a = strtol(p, &endptr, is_self->base);
        if (*endptr != '\0')
        {
            snprintf(err_buf, sizeof err_buf, "[%d]:%s is not a int", i, p);
            err_msg = err_buf;
            break;
        }
        arr[i] = a;
    }
    if ( ! err_msg && is_self->validator)
    {
        err_msg = is_self->validator(arr, i, is_self->context);
    }
    free(buf);
    free(arr);
    return err_msg;
}

char const* value_ints_parse(OptionValue self, char const* arg)
{
    struct opt_value_ints* is_self = value_ints_cast(self);
    char * buf = strdup(arg);
    int i ;
    char* p;
    for(p = strtok(buf, is_self->dlms),i = 0; p ; p = strtok(NULL, is_self->dlms), ++i)
    {
        int a = strtol(p, NULL, is_self->base);
        ((int*)self->pointer)[i] = a;
    }
    *is_self->output_n = i;
    free(buf);
    return NULL;
}

static void value_ints_store_default(OptionValue self)
{
    struct opt_value_ints* is_self = value_ints_cast(self);
    int n;
    if ( is_self->output_n == NULL ) n = is_self->default_n;
    else n = is_self->max_output_n < is_self->default_n ?is_self->max_output_n:is_self->default_n;
    memcpy(self->pointer, is_self->default_ints, n * sizeof n);
    if ( is_self->output_n ) *is_self->output_n = n;
}

static char const* value_ints_display_default(OptionValue self)
{
    static char str[200];
    struct opt_value_ints* is_self = value_ints_cast(self);
    char dlm = *is_self->dlms;
    int n = is_self->default_n;
    int const* arr = is_self->default_ints;
    int total = sizeof str;
    int writed = 0;
    for(int i = 0; i < n; ++i)
    {
        writed += snprintf(str + writed, total - writed, "%d", arr[i]);
        if (i != n-1)
            writed += snprintf(str + writed, total - writed, "%c", dlm);
    }
    return str;
}
static void value_ints_free(OptionValue self)
{
    struct opt_value_ints* is_self = value_ints_cast(self);
    if ( is_self->context && is_self->context_free )
        is_self->context_free(is_self->context);
    free(is_self);
}

struct option_value_ops g_value_ints_ops = {
    .validate = & value_ints_validate,
    .parse = & value_ints_parse,
    .store_default = & value_ints_store_default,
    .display_default = & value_ints_display_default,
    .free = & value_ints_free
};

static OptIntsBuilderInterface ints_set_required();
static OptIntsBuilderInterface ints_set_default_value(int const*, int);
static OptIntsBuilderInterface ints_set_delimiters(char const*);
static OptIntsBuilderInterface ints_set_validator(char const* (*)(int const*, int , void*));
static OptIntsBuilderInterface ints_set_context(void*);
static OptIntsBuilderInterface ints_set_context_free(void(*)(void*));
static OptIntsBuilderInterface ints_set_base(int n);
struct opt_ints_builder_interface g_ints_builder = {
    ._data = NULL,
    .required = &ints_set_required,
    .default_value = &ints_set_default_value,
    .delimiters = &ints_set_delimiters,
    .validator = &ints_set_validator,
    .context = &ints_set_context,
    .free = &ints_set_context_free,
    .base = &ints_set_base,
};
#define this_ints_value() cast(struct opt_value_ints*, g_ints_builder._data)
OptIntsBuilderInterface opt_ints(int * pointer, int* n)
{
    struct opt_value_ints * ints = (struct opt_value_ints*)malloc(sizeof *ints); 
    bzero(ints, sizeof *ints);
    ints->_base.pointer = pointer;
    assert(n);
    ints->max_output_n = *n;
    ints->output_n = n;
    ints->dlms = ",";
    ints->_base.ops = &g_value_ints_ops;
    g_ints_builder._data = &ints->_base;
    return &g_ints_builder;
}
static OptIntsBuilderInterface ints_set_required()
{
    this_ints_value()->_base.required = 1;
    return &g_ints_builder;
}

static OptIntsBuilderInterface ints_set_default_value(int const* arr, int n)
{
    this_ints_value()->default_n = n;
    this_ints_value()->default_ints = arr;
    this_ints_value()->_base.has_default = 1;
    return &g_ints_builder;
}

static OptIntsBuilderInterface ints_set_delimiters(char const* dlms)
{
    this_ints_value()->dlms = dlms;
    return &g_ints_builder;
}

static OptIntsBuilderInterface ints_set_validator(char const* (validator)(int const*, int, void*))
{
    this_ints_value()->validator = validator;
    return &g_ints_builder;
}

static OptIntsBuilderInterface ints_set_context(void* context)
{
    this_ints_value()->context = context;
    return &g_ints_builder;
}

static OptIntsBuilderInterface ints_set_context_free(void(*on_free)(void*))
{
    this_ints_value()->context_free = on_free;
    return &g_ints_builder;
}

static OptIntsBuilderInterface ints_set_base(int n)
{
    this_ints_value()->base = n;
    return &g_ints_builder;
}
// }}}
// option value doubles{{{
struct opt_value_doubles
{
    struct option_value _base;
    int max_output_n;
    int* output_n;
    double const* default_doubles;
    int default_n;
    char const* dlms;
    char const* (*validator)(double const*, int, void*);
    void* context;
    void (*context_free)(void*);
};
#define value_doubles_cast(self) cast(struct opt_value_doubles*, self)
char const* value_doubles_validate(OptionValue self, char const* arg)
{
    struct opt_value_doubles* is_self = value_doubles_cast(self);
    double * arr = (double*)malloc(sizeof(double)*is_self->max_output_n);
    char* buf = strdup(arg);
    static char err_buf[100];
    int i;
    char* p;
    char const* err_msg = NULL;
    for(p = strtok(buf, is_self->dlms),i = 0; p; p = strtok(NULL, is_self->dlms), ++i)
    {
        if ( i ==  is_self->max_output_n)
        {
            err_msg = "too much double";
            break;
        }
        char * endptr;
        double a = strtod(p, &endptr);//strtol(p, &endptr, is_self->base);
        if (*endptr != '\0')
        {
            snprintf(err_buf, sizeof err_buf, "[%d]:%s is not a double", i, p);
            err_msg = err_buf;
            break;
        }
        arr[i] = a;
    }
    if ( ! err_msg && is_self->validator)
    {
        err_msg = is_self->validator(arr, i, is_self->context);
    }
    free(buf);
    free(arr);
    return err_msg;
}
char const* value_doubles_parse(OptionValue self, char const* arg)
{
    struct opt_value_doubles* is_self = value_doubles_cast(self);
    char * buf = strdup(arg);
    int i ;
    char* p;
    for(p = strtok(buf, is_self->dlms),i = 0; p ; p = strtok(NULL, is_self->dlms), ++i)
    {
        double a = strtod(p, NULL);//strtol(p, NULL, is_self->base);
        ((double*)self->pointer)[i] = a;
    }
    *is_self->output_n = i;
    free(buf);
    return NULL;
}
static void value_doubles_store_default(OptionValue self)
{
    struct opt_value_doubles* is_self = value_doubles_cast(self);
    int n;
    if ( is_self->output_n == NULL ) n = is_self->default_n;
    else n = is_self->max_output_n < is_self->default_n ?is_self->max_output_n:is_self->default_n;
    memcpy(self->pointer, is_self->default_doubles, n * sizeof(double));
    if ( is_self->output_n ) *is_self->output_n = n;
}
static char const* value_doubles_display_default(OptionValue self)
{
    static char str[200];
    struct opt_value_doubles* is_self = value_doubles_cast(self);
    char dlm = *is_self->dlms;
    int n = is_self->default_n;
    double const* arr = is_self->default_doubles;
    int total = sizeof str;
    int writed = 0;
    for(int i = 0; i < n; ++i)
    {
        writed += snprintf(str + writed, total - writed, "%g", arr[i]);
        if (i != n-1)
            writed += snprintf(str + writed, total - writed, "%c", dlm);
    }
    return str;
}
static void value_doubles_free(OptionValue self)
{
    struct opt_value_doubles* is_self = value_doubles_cast(self);
    if ( is_self->context && is_self->context_free )
        is_self->context_free(is_self->context);
    free(is_self);
}

struct option_value_ops g_value_doubles_ops = {
    .validate = & value_doubles_validate,
    .parse = & value_doubles_parse,
    .store_default = & value_doubles_store_default,
    .display_default = & value_doubles_display_default,
    .free = & value_doubles_free
};

static OptDoublesBuilderInterface doubles_set_required();
static OptDoublesBuilderInterface doubles_set_default_value(double const*, int);
static OptDoublesBuilderInterface doubles_set_delimiters(char const*);
static OptDoublesBuilderInterface doubles_set_validator(char const* (*)(double const*, int , void*));
static OptDoublesBuilderInterface doubles_set_context(void*);
static OptDoublesBuilderInterface doubles_set_context_free(void(*)(void*));
struct opt_doubles_builder_interface g_doubles_builder = {
    ._data = NULL,
    .required = &doubles_set_required,
    .default_value = &doubles_set_default_value,
    .delimiters = &doubles_set_delimiters,
    .validator = &doubles_set_validator,
    .context = &doubles_set_context,
    .free = &doubles_set_context_free,
};
#define this_doubles_value() cast(struct opt_value_doubles*, g_doubles_builder._data)

OptDoublesBuilderInterface opt_doubles(double * pointer, int* n)
{
    struct opt_value_doubles * doubles = (struct opt_value_doubles*)malloc(sizeof *doubles); 
    bzero(doubles, sizeof *doubles);
    doubles->_base.pointer = pointer;
    assert(n);
    doubles->max_output_n = *n;
    doubles->output_n = n;
    doubles->dlms = ",";
    doubles->_base.ops = &g_value_doubles_ops;
    g_doubles_builder._data = &doubles->_base;
    return &g_doubles_builder;
}
static OptDoublesBuilderInterface doubles_set_required()
{
    this_doubles_value()->_base.required = 1;
    return &g_doubles_builder;
}

static OptDoublesBuilderInterface doubles_set_default_value(double const* arr, int n)
{
    this_doubles_value()->default_n = n;
    this_doubles_value()->default_doubles = arr;
    this_doubles_value()->_base.has_default = 1;
    return &g_doubles_builder;
}

static OptDoublesBuilderInterface doubles_set_delimiters(char const* dlms)
{
    this_doubles_value()->dlms = dlms;
    return &g_doubles_builder;
}

static OptDoublesBuilderInterface doubles_set_validator(char const* (validator)(double const*, int, void*))
{
    this_doubles_value()->validator = validator;
    return &g_doubles_builder;
}

static OptDoublesBuilderInterface doubles_set_context(void* context)
{
    this_doubles_value()->context = context;
    return &g_doubles_builder;
}

static OptDoublesBuilderInterface doubles_set_context_free(void(*on_free)(void*))
{
    this_doubles_value()->context_free = on_free;
    return &g_doubles_builder;
}
// }}}
// option strings {{{
struct opt_value_strings
{
    struct option_value _base;
    int max_output_n;
    int* output_n;
    char const** default_strings;
    int default_n;
    char const* dlms;
    char* buf;
};
#define value_strings_cast(self) cast(struct opt_value_strings*, self)
char const* value_strings_validate(OptionValue self, char const* arg)
{
    char* buf = strdup(arg);
    struct opt_value_strings* ss_self = value_strings_cast(self);
    int n = ss_self->max_output_n;
    int i = 0;
    for(char* p = strtok(buf, ss_self->dlms); p ; p =strtok(NULL, ss_self->dlms))
    {
       ++i; 
    }
    char const* err_msg = NULL;
    if (i > n)
    {
        err_msg = "too much strings";
    }
    free(buf);
    return err_msg;
}
char const* value_strings_parse(OptionValue self, char const* arg)
{
    struct opt_value_strings* ss_self = value_strings_cast(self);
    if (ss_self->buf) free(ss_self->buf);
    if ( self->pointer )
    {
        ss_self->buf = strdup(arg);
        char const** output = (char const**)self->pointer;
        int i = 0;
        for(char* p = strtok(ss_self->buf, ss_self->dlms); p; p = strtok(NULL, ss_self->dlms))
        {
            output[i++] = p;
        }
        *(ss_self->output_n) = i;
    }
    return NULL;
}
static void value_strings_store_default(OptionValue self)
{
    struct opt_value_strings* ss_self = value_strings_cast(self);
    int n = ss_self->max_output_n < ss_self->default_n ?ss_self->max_output_n:ss_self->default_n;
    memcpy(self->pointer, ss_self->default_strings, n * sizeof(char const*));
    *(ss_self->output_n) = n;
}
static char const* value_strings_display_default(OptionValue self)
{
    static char str[200];
    struct opt_value_strings* ss_self = value_strings_cast(self);
    char dlm = *ss_self->dlms;
    int n = ss_self->default_n;
    char const** arr = ss_self->default_strings;
    int total = sizeof str;
    int writed = 0;
    for(int i = 0; i < n; ++i)
    {
        writed += snprintf(str + writed, total - writed, "%s", arr[i]);
        if (i != n-1)
            writed += snprintf(str + writed, total - writed, "%c", dlm);
    }
    return str;
}
static void value_strings_free(OptionValue self)
{
    struct opt_value_strings* ss_self = value_strings_cast(self);
    if ( ss_self->buf ) free(ss_self->buf);
    if ( ss_self->default_strings ) free((ss_self->default_strings));
}

struct option_value_ops g_value_strings_ops = {
    .validate = & value_strings_validate,
    .parse = & value_strings_parse,
    .store_default = & value_strings_store_default,
    .display_default = & value_strings_display_default,
    .free = & value_strings_free
};

static OptStringsBuilderInterface strings_set_required();
static OptStringsBuilderInterface strings_set_default_value(char const* arg, ...);
static OptStringsBuilderInterface strings_set_delimiters(char const*);
struct opt_strings_builder_interface g_strings_builder = {
    ._data = NULL,
    .required = &strings_set_required,
    .default_value = &strings_set_default_value,
    .delimiters = &strings_set_delimiters,
};
#define this_strings_value() cast(struct opt_value_strings*, g_strings_builder._data)

OptStringsBuilderInterface opt_strings(char const** pointer, int* n)
{
    assert(n);
    struct opt_value_strings * strings= (struct opt_value_strings*)malloc(sizeof *strings);
    bzero(strings, sizeof *strings);
    strings->_base.pointer = pointer;
    strings->max_output_n = *n;
    strings->output_n = n;
    strings->dlms = ",";
    strings->_base.ops = &g_value_strings_ops;
    g_strings_builder._data = &strings->_base;
    return &g_strings_builder;
}
static OptStringsBuilderInterface strings_set_required()
{
    this_strings_value()->_base.required = 1;
    return &g_strings_builder;
}

static OptStringsBuilderInterface strings_set_default_value(char const* arg, ...)
{
    assert(arg);
    int n = 1;
    va_list parg;
    char const* str;
    for(va_start(parg, arg), str = va_arg(parg, char const*); str; str = va_arg(parg, char const*))
    {
        ++n;
    }
    this_strings_value()->default_n = n;
    this_strings_value()->default_strings = (char const**)malloc(n * sizeof(char const*));
    this_strings_value()->_base.has_default = 1;
    va_start(parg, arg);
    str = arg;
    int i = 0;
    for(str = arg, va_start(parg, arg); str; str = va_arg(parg, char const*), ++i)
    {
        this_strings_value()->default_strings[i] = str;
    }
    return &g_strings_builder;
}

static OptStringsBuilderInterface strings_set_delimiters(char const* dlms)
{
    this_strings_value()->dlms = dlms;
    return &g_strings_builder;
}
//}}}
