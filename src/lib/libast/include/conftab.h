#ifndef _CONFTAB_H
#define _CONFTAB_H

#include <config_ast.h>

#if !defined(const) && !defined(__STDC__) && !defined(__cplusplus) && !defined(c_plusplus)
#define const
#endif

#define conf _ast_conf_data
#define conf_elements _ast_conf_ndata

#define prefix _ast_conf_prefix
#define prefix_elements _ast_conf_nprefix

#define CONF_nop 0
#define CONF_confstr 1
#define CONF_pathconf 2
#define CONF_sysconf 3
#define CONF_sysinfo 4

#define CONF_C 0
#define CONF_SUN 1
#define CONF_POSIX 2
#define CONF_SVID 3
#define CONF_XOPEN 4
#define CONF_SCO 5
#define CONF_AST 6
#define CONF_AES 7
#define CONF_XPG 8
#define CONF_GNU 9
#define CONF_TRUSTEDBSD 10
#define CONF_call 11

#define _pth_getconf "/usr/bin/getconf"
#define _pth_getconf_a "-a"

#define CONF_DEFER_CALL 0x0001
#define CONF_DEFER_MM 0x0002
#define CONF_FEATURE 0x0004
#define CONF_LIMIT 0x0008
#define CONF_LIMIT_DEF 0x0010
#define CONF_MINMAX 0x0020
#define CONF_MINMAX_DEF 0x0040
#define CONF_NOSECTION 0x0080
#define CONF_NOUNDERSCORE 0x0100
#define CONF_PREFIX_ONLY 0x0200
#define CONF_PREFIXED 0x0400
#define CONF_STANDARD 0x0800
#define CONF_STRING 0x1000
#define CONF_UNDERSCORE 0x2000
#define CONF_USER 0x4000

struct Conf_s;
typedef struct Conf_s Conf_t;

typedef struct Value_s {
    int64_t number;
    const char *string;
} Value_t;

struct Conf_s {
    const char name[32];
    Value_t limit;
    Value_t minmax;
    unsigned int flags;
    short standard;
    short section;
    short call;
    short op;
};

typedef struct Prefix_s {
    const char name[16];
    short length;
    short standard;
    short call;
} Prefix_t;

extern const Conf_t conf[];
extern const int conf_elements;

extern const Prefix_t prefix[];
extern const int prefix_elements;

#endif
