#include "package.h"
#include "extconf.h"

#include <postgres.h>
#include <catalog/pg_type.h>
#include <utils/builtins.h>
#include <utils/timestamp.h>
#include <lib/stringinfo.h>
#include <ruby.h>

#define CPY_FREE(p0_, p1_, size_) do {		\
    void *p2_ = (void *)p1_;			\
    memcpy((p0_), (p2_), (size_));		\
    pfree(p2_);					\
} while (0)

#if PG_PL_VERSION >= 74

#define PL_MDUMP(name_,func_)                                   \
static VALUE                                                    \
name_(int argc, VALUE *argv, VALUE obj)                         \
{                                                               \
    void *mac;                                                  \
    char *res;                                                  \
    VALUE result;                                               \
                                                                \
    Data_Get_Struct(obj, void, mac);                            \
    res = (char *)PLRUBY_DFC1(func_, mac);                      \
    result = rb_tainted_str_new(VARDATA(res), VARSIZE(res));    \
    pfree(res);                                                 \
    return result;                                              \
}

#define PL_MLOAD(name_,func_,type_)                                     \
static VALUE                                                            \
name_(VALUE obj, VALUE a)                                               \
{                                                                       \
    StringInfoData si;                                                  \
    type_ *mac0, *mac1;                                                 \
                                                                        \
    if (TYPE(a) != T_STRING || !RSTRING_LEN(a)) {                      \
        rb_raise(rb_eArgError, "expected a String object");             \
    }                                                                   \
    initStringInfo(&si);                                                \
    appendBinaryStringInfo(&si, RSTRING_PTR(a), RSTRING_LEN(a));      \
    mac1 = (type_ *)PLRUBY_DFC1(func_, &si);                            \
    pfree(si.data);                                                     \
    Data_Get_Struct(obj, type_, mac0);                                  \
    CPY_FREE(mac0, mac1, sizeof(type_));                                \
    return obj;                                                         \
}

#define PL_MLOADVAR(name_,func_,type_,size_)                            \
static VALUE                                                            \
name_(VALUE obj, VALUE a)                                               \
{                                                                       \
    StringInfoData si;                                                  \
    type_ *mac0, *mac1;                                                 \
    int szl;                                                            \
                                                                        \
    if (TYPE(a) != T_STRING || !RSTRING_LEN(a)) {                      \
        rb_raise(rb_eArgError, "expected a String object");             \
    }                                                                   \
    initStringInfo(&si);                                                \
    appendBinaryStringInfo(&si, RSTRING_PTR(a), RSTRING_LEN(a));      \
    mac1 = (type_ *)PLRUBY_DFC1(func_, &si);                            \
    pfree(si.data);                                                     \
    Data_Get_Struct(obj, type_, mac0);                                  \
    free(mac0);                                                         \
    szl = size_(mac1);                                                  \
    mac0 = (type_ *)ALLOC_N(char, szl);                                 \
    CPY_FREE(mac0, mac1, szl);                                          \
    RDATA(obj)->data = mac0;                                            \
    return obj;                                                         \
}

#ifndef RUBY_CAN_USE_MARSHAL_LOAD
extern VALUE plruby_s_load _((VALUE, VALUE));
#endif

#else

#define PL_MDUMP(name_,func_)
#define PL_MLOAD(name_,func_,type_) 
#define PL_MLOADVAR(name_,func_,type_,size_)

#endif

extern VALUE plruby_to_s _((VALUE));
extern VALUE plruby_s_new _((int, VALUE *, VALUE));
extern VALUE plruby_define_void_class _((char *, char *));
#ifndef HAVE_RB_INITIALIZE_COPY
extern VALUE plruby_clone _((VALUE));
#endif
extern Oid plruby_datum_oid _((VALUE, int *));
extern VALUE plruby_datum_set _((VALUE, Datum));
extern VALUE plruby_datum_get _((VALUE, Oid *));

#ifndef StringValuePtr
#define StringValuePtr(x) STR2CSTR(x)
#endif

#ifndef RSTRING_PTR
# define RSTRING_PTR(x_) RSTRING(x_)->ptr
# define RSTRING_LEN(x_) RSTRING(x_)->len
#endif

#ifndef RARRAY_PTR
# define RARRAY_PTR(x_) RARRAY(x_)->ptr
# define RARRAY_LEN(x_) RARRAY(x_)->len
#endif

#ifndef RHASH_TBL
#define RHASH_TBL(x_) (RHASH(x_)->tbl)
#endif

#ifndef RFLOAT_VALUE
#define RFLOAT_VALUE(x_) (RFLOAT(x_)->value)
#endif

#ifndef HAVE_RB_FRAME_THIS_FUNC
# define rb_frame_this_func rb_frame_last_func
#endif

extern Datum plruby_dfc0 _((PGFunction));
extern Datum plruby_dfc1 _((PGFunction, Datum));
extern Datum plruby_dfc2 _((PGFunction, Datum, Datum));
extern Datum plruby_dfc3 _((PGFunction, Datum, Datum, Datum));

#define PLRUBY_DFC0(a_) plruby_dfc0(a_)
#define PLRUBY_DFC1(a_,b_) plruby_dfc1(a_,PointerGetDatum(b_))
#define PLRUBY_DFC2(a_,b_,c_) plruby_dfc2(a_,PointerGetDatum(b_),PointerGetDatum(c_))
#define PLRUBY_DFC3(a_,b_,c_,d_) plruby_dfc3(a_,PointerGetDatum(b_),PointerGetDatum(c_),PointerGetDatum(d_))
