#include <config_ast.h>
#include <ast_standards.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <limits.h>
#include <unistd.h>
{headers}
#include <sys/param.h>
#include <sys/stat.h>
#include "conftab.h"

//
// Prefix strings -- the first few are indexed by Conf_t.standard.
//
const Prefix_t prefix[] = {{
{prefix_standards}
    {{ "XX",         2,      CONF_POSIX,     CONF_nop }},
    {{ "CS",         2,      CONF_POSIX,     CONF_confstr }},
    {{ "PC",         2,      CONF_POSIX,     CONF_pathconf }},
    {{ "SC",         2,      CONF_POSIX,     CONF_sysconf }},
    {{ "SI",         2,      CONF_SVID,      CONF_sysinfo }},
}};

const int prefix_elements = (int)sizeof(prefix) / (int)sizeof(prefix[0]);

//
// Conf strings sorted in ascending order.
//
const Conf_t conf[] = {{
{getconf}
}};

const int conf_elements = (int)sizeof(conf) / (int)sizeof(conf[0]);
