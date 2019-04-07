// This contains functions useful for debugging ksh code. For example, `_dprint_vtp()` to write
// information about a struct Value to stderr.
//
#include "config_ast.h"  // IWYU pragma: keep

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>

#include "ast_assert.h"
#include "name.h"

// Number of spaces to indent each level of nested data structures.
#define INDENT 2
#define MAX_LEVELS 5

// This is to give unit tests some control over the addresses displayed. Setting this to the
// address of the first var in the unit test module will cause all pointers to be displayed as
// offsets to that var.
void *_dprint_vt_base_addr = NULL;
#define BASE_ADDR(p) (char *)((char *)(p) - (char *)_dprint_vt_base_addr)

// Max number of pointers we remember when following a cycle of pointers.
#define MAX_PTRS 100
static void *ptrs_seen[MAX_PTRS];
static int next_ptr = 0;
static bool max_ptrs_warned = false;

// When dumping an object (e.g., a Namval_t) at level zero forget about any pointers we've seen from
// previous debug print calls.
static_fn void clear_ptrs() {
    next_ptr = 0;
    max_ptrs_warned = false;
}

// Return true if we've already seen the pointer; otherwise, remember it.
static_fn bool ptr_seen(void *p) {
    if (next_ptr == MAX_PTRS) {
        if (!max_ptrs_warned) DPRINTF("Too many pointers already cached when checking %p", p);
        return true;  // return true to avoid cycles we can't detect -- should never happen
    }

    for (int i = 0; i < next_ptr; i++) {
        if (ptrs_seen[i] == p) return true;
    }
    ptrs_seen[next_ptr++] = p;
    return false;
}

static_fn const char *indent(int level, const char *fmt) {
    static char buf[MAX_LEVELS * INDENT + 1 + 100];

    if (level == 0) return fmt;

    if (level <= MAX_LEVELS) {
        memset(buf, ' ', level * INDENT);
        buf[level * INDENT] = '\0';
    } else {
        memset(buf, '>', MAX_LEVELS * INDENT);
        buf[MAX_LEVELS * INDENT] = '\0';
    }
    strlcat(buf, fmt, sizeof(buf));
    return buf;
}

typedef void(vtp_dprintf)(const char *, int, const char *, int, const char *, const struct Value *);

static_fn void _dprint_VT_do_not_use(const char *file_name, int lineno, const char *func_name,
                                     int level, const char *var_name, const struct Value *vtp) {
    UNUSED(vtp);
    UNUSED(var_name);
    _dprintf(file_name, lineno, func_name, indent(level, "undefined (type is VT_do_not_use)"));
}

static_fn void _dprint_VT_vp(const char *file_name, int lineno, const char *func_name, int level,
                             const char *var_name, const struct Value *vtp) {
    UNUSED(var_name);
    _dprintf(file_name, lineno, func_name, indent(level, "void* %p"),
             BASE_ADDR(FETCH_VTP(vtp, vp)));
}

static_fn void _dprint_VT_cp(const char *file_name, int lineno, const char *func_name, int level,
                             const char *var_name, const struct Value *vtp) {
    UNUSED(var_name);
    _dprintf(file_name, lineno, func_name, indent(level, "char* %p |%s|"),
             BASE_ADDR(FETCH_VTP(vtp, cp)), FETCH_VTP(vtp, cp));
}

static_fn void _dprint_VT_const_cp(const char *file_name, int lineno, const char *func_name,
                                   int level, const char *var_name, const struct Value *vtp) {
    UNUSED(var_name);
    _dprintf(file_name, lineno, func_name, indent(level, "const char* %p |%s|"),
             BASE_ADDR(FETCH_VTP(vtp, const_cp)), FETCH_VTP(vtp, const_cp));
}

static_fn void _dprint_VT_uc(const char *file_name, int lineno, const char *func_name, int level,
                             const char *var_name, const struct Value *vtp) {
    UNUSED(var_name);
    _dprintf(file_name, lineno, func_name, indent(level, "unsigned char %hhu (0x%hhX)"),
             FETCH_VTP(vtp, uc), FETCH_VTP(vtp, uc));
}

static_fn void _dprint_VT_pp(const char *file_name, int lineno, const char *func_name, int level,
                             const char *var_name, const struct Value *vtp) {
    UNUSED(var_name);
    _dprintf(file_name, lineno, func_name, indent(level, "char** %p"),
             BASE_ADDR(FETCH_VTP(vtp, pp)));
}

static_fn void _dprint_VT_h(const char *file_name, int lineno, const char *func_name, int level,
                            const char *var_name, const struct Value *vtp) {
    UNUSED(var_name);
    _dprintf(file_name, lineno, func_name, indent(level, "short %hi (0x%hX)"), FETCH_VTP(vtp, h),
             FETCH_VTP(vtp, h));
}

static_fn void _dprint_VT_i(const char *file_name, int lineno, const char *func_name, int level,
                            const char *var_name, const struct Value *vtp) {
    UNUSED(var_name);
    _dprintf(file_name, lineno, func_name, indent(level, "int %i (0x%X)"), FETCH_VTP(vtp, i),
             FETCH_VTP(vtp, i));
}

static_fn void _dprint_VT_l(const char *file_name, int lineno, const char *func_name, int level,
                            const char *var_name, const struct Value *vtp) {
    UNUSED(var_name);
    _dprintf(file_name, lineno, func_name, indent(level, "int %li (0x%lX)"), FETCH_VTP(vtp, l),
             FETCH_VTP(vtp, l));
}

static_fn void _dprint_VT_d(const char *file_name, int lineno, const char *func_name, int level,
                            const char *var_name, const struct Value *vtp) {
    UNUSED(var_name);
    _dprintf(file_name, lineno, func_name, indent(level, "double %g"), FETCH_VTP(vtp, d));
}

static_fn void _dprint_VT_f(const char *file_name, int lineno, const char *func_name, int level,
                            const char *var_name, const struct Value *vtp) {
    UNUSED(var_name);
    _dprintf(file_name, lineno, func_name, indent(level, "float %g"), FETCH_VTP(vtp, f));
}

static_fn void _dprint_VT_i16(const char *file_name, int lineno, const char *func_name, int level,
                              const char *var_name, const struct Value *vtp) {
    UNUSED(var_name);
    _dprintf(file_name, lineno, func_name, indent(level, "int16_t %" PRIi16 "(0x%" PRIX16 ")"),
             FETCH_VTP(vtp, i16), FETCH_VTP(vtp, i16));
}

static_fn void _dprint_VT_ip(const char *file_name, int lineno, const char *func_name, int level,
                             const char *var_name, const struct Value *vtp) {
    UNUSED(var_name);
    _dprintf(file_name, lineno, func_name, indent(level, "int* %p => %i (0x%X)"),
             BASE_ADDR(FETCH_VTP(vtp, ip)), *FETCH_VTP(vtp, ip), *FETCH_VTP(vtp, ip));
}

static_fn void _dprint_VT_i16p(const char *file_name, int lineno, const char *func_name, int level,
                               const char *var_name, const struct Value *vtp) {
    UNUSED(var_name);
    _dprintf(file_name, lineno, func_name,
             indent(level, "int16_t* %p => %" PRIi16 " (0x%" PRIX16 ")"),
             BASE_ADDR(FETCH_VTP(vtp, i16p)), *FETCH_VTP(vtp, i16p), *FETCH_VTP(vtp, i16p));
}

static_fn void _dprint_VT_i32p(const char *file_name, int lineno, const char *func_name, int level,
                               const char *var_name, const struct Value *vtp) {
    UNUSED(var_name);
    _dprintf(file_name, lineno, func_name,
             indent(level, "int32_t* %p => %" PRIi32 " (0x%" PRIX32 ")"),
             BASE_ADDR(FETCH_VTP(vtp, i32p)), *FETCH_VTP(vtp, i32p), *FETCH_VTP(vtp, i32p));
}

static_fn void _dprint_VT_i64p(const char *file_name, int lineno, const char *func_name, int level,
                               const char *var_name, const struct Value *vtp) {
    UNUSED(var_name);
    _dprintf(file_name, lineno, func_name,
             indent(level, "int64_t* %p => %" PRIi64 " (0x%" PRIX64 ")"),
             BASE_ADDR(FETCH_VTP(vtp, i64p)), *FETCH_VTP(vtp, i64p), *FETCH_VTP(vtp, i64p));
}

static_fn void _dprint_VT_dp(const char *file_name, int lineno, const char *func_name, int level,
                             const char *var_name, const struct Value *vtp) {
    UNUSED(var_name);
    _dprintf(file_name, lineno, func_name, indent(level, "double* %p => %g"),
             BASE_ADDR(FETCH_VTP(vtp, dp)), *FETCH_VTP(vtp, dp));
}

static_fn void _dprint_VT_fp(const char *file_name, int lineno, const char *func_name, int level,
                             const char *var_name, const struct Value *vtp) {
    UNUSED(var_name);
    _dprintf(file_name, lineno, func_name, indent(level, "float* %p => %g"),
             BASE_ADDR(FETCH_VTP(vtp, fp)), *FETCH_VTP(vtp, fp));
}

static_fn void _dprint_VT_sfdoublep(const char *file_name, int lineno, const char *func_name,
                                    int level, const char *var_name, const struct Value *vtp) {
    UNUSED(var_name);
    _dprintf(file_name, lineno, func_name, indent(level, "Sfdouble_t* %p => %Lg"),
             BASE_ADDR(FETCH_VTP(vtp, sfdoublep)), *FETCH_VTP(vtp, sfdoublep));
}

static_fn void _dprint_VT_np(const char *file_name, int lineno, const char *func_name, int level,
                             const char *var_name, const struct Value *vtp) {
    char buf[128];

    strlcpy(buf, var_name, sizeof(buf));
    strlcat(buf, ".np", sizeof(buf));
    void *p = FETCH_VTP(vtp, np);
    if (ptr_seen(p)) {
        _dprintf(file_name, lineno, func_name,
                 indent(level, "WARN: %s ptr %p already dumped; not following it"), buf, p);
        return;
    }
    _dprint_nvp(file_name, lineno, func_name, level, buf, p);
}

static_fn void _dprint_VT_up(const char *file_name, int lineno, const char *func_name, int level,
                             const char *var_name, const struct Value *vtp) {
    char buf[128];

    strlcpy(buf, var_name, sizeof(buf));
    strlcat(buf, ".up", sizeof(buf));
    // This is a slightly unusual case that doesn't follow the other patterns for printing an
    // embedded struct pointer; e.g., _dprint_VT_nrp. That is because this is printing a pointer to
    // another struct Value. So to provide context we need to print that pointer addr now before
    // printing info about the struct Value that pointer refers to.
    void *p = FETCH_VTP(vtp, up);
    _dprintf(file_name, lineno, func_name, indent(level, "struct Value %s %p is..."), buf,
             BASE_ADDR(p));
    if (ptr_seen(p)) {
        _dprintf(file_name, lineno, func_name,
                 indent(level, "WARN: %s ptr %p already dumped; not following it"), buf, p);
        return;
    }
    _dprint_vtp(file_name, lineno, func_name, level + 1, buf, p);
}

static_fn void _dprint_VT_rp(const char *file_name, int lineno, const char *func_name, int level,
                             const char *var_name, const struct Value *vtp) {
    UNUSED(var_name);
    _dprintf(file_name, lineno, func_name, indent(level, "struct Ufunction* %p"),
             BASE_ADDR(FETCH_VTP(vtp, rp)));
}

static_fn void _dprint_VT_funp(const char *file_name, int lineno, const char *func_name, int level,
                               const char *var_name, const struct Value *vtp) {
    UNUSED(var_name);
    _dprintf(file_name, lineno, func_name, indent(level, "struct Namfun* %p"),
             BASE_ADDR(FETCH_VTP(vtp, funp)));
}

static_fn void _dprint_VT_nrp(const char *file_name, int lineno, const char *func_name, int level,
                              const char *var_name, const struct Value *vtp) {
    char buf[128];

    strlcpy(buf, var_name, sizeof(buf));
    strlcat(buf, ".nrp", sizeof(buf));
    void *p = FETCH_VTP(vtp, nrp);
    if (ptr_seen(p)) {
        _dprintf(file_name, lineno, func_name,
                 indent(level, "WARN: %s ptr %p already dumped; not following it"), buf, p);
        return;
    }
    _dprint_nrp(file_name, lineno, func_name, level, buf, p);
}

static_fn void _dprint_VT_shbltinp(const char *file_name, int lineno, const char *func_name,
                                   int level, const char *var_name, const struct Value *vtp) {
    UNUSED(var_name);
    _dprintf(file_name, lineno, func_name, indent(level, "struct Shbltin_f %p"),
             BASE_ADDR(FETCH_VTP(vtp, shbltinp)));
}

static_fn void _dprint_VT_pathcomp(const char *file_name, int lineno, const char *func_name,
                                   int level, const char *var_name, const struct Value *vtp) {
    UNUSED(var_name);
    _dprintf(file_name, lineno, func_name, indent(level, "struct pathcomp* %p"),
             BASE_ADDR(FETCH_VTP(vtp, pathcomp)));
}

static_fn void _dprint_VT_pidp(const char *file_name, int lineno, const char *func_name, int level,
                               const char *var_name, const struct Value *vtp) {
    UNUSED(var_name);
    _dprintf(file_name, lineno, func_name, indent(level, "pid_t* %p => %" PRIu64),
             BASE_ADDR(FETCH_VTP(vtp, pidp)), (uint64_t)*FETCH_VTP(vtp, pidp));
}

static_fn void _dprint_VT_uidp(const char *file_name, int lineno, const char *func_name, int level,
                               const char *var_name, const struct Value *vtp) {
    UNUSED(var_name);
    _dprintf(file_name, lineno, func_name, indent(level, "uid_t* %p => %" PRIu64),
             BASE_ADDR(FETCH_VTP(vtp, uidp)), (uint64_t)*FETCH_VTP(vtp, uidp));
}

static_fn void _dprint_VT_sentinal(const char *file_name, int lineno, const char *func_name,
                                   int level, const char *var_name, const struct Value *vtp) {
    UNUSED(file_name);
    UNUSED(lineno);
    UNUSED(func_name);
    UNUSED(level);
    UNUSED(var_name);
    UNUSED(vtp);
}

// This must be kept in the same order as the `enum value_type` in "name.h".
static vtp_dprintf *dprint_vtp_dispatch[] = {
    _dprint_VT_do_not_use,  // VT_do_not_use
    _dprint_VT_vp,          // VT_vp
    _dprint_VT_cp,          // VT_cp
    _dprint_VT_const_cp,    // VT_const_cp
    _dprint_VT_pp,          // VT_pp
    _dprint_VT_uc,          // VT_uc
    _dprint_VT_h,           // VT_h
    _dprint_VT_i,           // VT_i
    _dprint_VT_l,           // VT_l
    _dprint_VT_d,           // VT_d
    _dprint_VT_f,           // VT_f
    _dprint_VT_i16,         // VT_i16
    _dprint_VT_ip,          // VT_ip
    _dprint_VT_i16p,        // VT_i16p
    _dprint_VT_i32p,        // VT_i32p
    _dprint_VT_i64p,        // VT_i64p
    _dprint_VT_dp,          // VT_dp
    _dprint_VT_fp,          // VT_fp
    _dprint_VT_sfdoublep,   // VT_sfdoublep
    _dprint_VT_np,          // VT_np
    _dprint_VT_up,          // VT_up
    _dprint_VT_rp,          // VT_rp
    _dprint_VT_funp,        // VT_funp
    _dprint_VT_nrp,         // VT_nrp
    _dprint_VT_shbltinp,    // VT_shbltinp
    _dprint_VT_pathcomp,    // VT_pathcomp
    _dprint_VT_pidp,        // VT_pidp
    _dprint_VT_uidp,        // VT_uidp
    _dprint_VT_sentinal     // must be last
};

// Diagnostic print a struct Value object.
void _dprint_vtp(const char *file_name, int const lineno, const char *func_name, int level,
                 const char *var_name, const void *vp) {
    if (!vp) {
        _dprintf(file_name, lineno, func_name, indent(level, "struct Value %s == NULL"), var_name);
        return;
    }

    if (level == 0) clear_ptrs();

    // We do this rather than a sizeof(dprint_vtp_dispatch) check because the latter is a constant
    // expression that causes lint warnings.
    const struct Value *vtp = vp;
    assert(dprint_vtp_dispatch[VT_sentinal] == _dprint_VT_sentinal);
    assert(vtp->type >= VT_do_not_use && vtp->type < VT_sentinal);

    if (vtp->type == VT_do_not_use) {
        _dprintf(file_name, lineno, func_name,
                 indent(level, "struct Value %s is undefined (type is VT_do_not_use)"), var_name);
        return;
    }

    _dprintf(file_name, lineno, func_name,
             indent(level, "struct Value %s.%s stored @ %s:%d in %s() is..."), var_name,
             value_type_names[vtp->type], vtp->filename ? vtp->filename : "undef", vtp->line_num,
             vtp->funcname ? vtp->funcname : "undef");
    (dprint_vtp_dispatch[vtp->type])(file_name, lineno, func_name, level + 1, var_name, vtp);
}

// If a unit test has set a base addr for struct Value objects just use a constant for the printed
// address of a Namval_t*. This is needed because different environments have different padding and
// alignment of these structures. That makes it impossible to emit a consistent address in the
// diagnostic message.
#define NP_BASE_ADDR(np) (_dprint_vt_base_addr ? (void *)0x88 : (void *)np)

// Diagnostic print a struct Namval (aka Namval_t) object.
void _dprint_nvp(const char *file_name, const int lineno, const char *func_name, int level,
                 const char *var_name, const void *vp) {
    const Namval_t *np = vp;

    if (level == 0) clear_ptrs();

    _dprintf(file_name, lineno, func_name, indent(level, "struct Namval %s @ %p"), var_name,
             NP_BASE_ADDR(np));
    if (!np) return;
    _dprintf(file_name, lineno, func_name, indent(level + 1, "->nvname %p |%s|"),
             NP_BASE_ADDR(np->nvname), np->nvname);
    _dprintf(file_name, lineno, func_name, indent(level + 1, "->nvsize %d"), np->nvsize);
    _dprintf(file_name, lineno, func_name, indent(level + 1, "->nvalue is..."));
    _dprint_vtp(file_name, lineno, func_name, level + 1, "", &np->nvalue);
    if (np->nvenv) {
        if (np->nvenv_is_cp) {
            _dprintf(file_name, lineno, func_name, indent(level + 1, "->nvenv %p |%s|"),
                     NP_BASE_ADDR(np->nvenv), np->nvenv);
        } else {
            _dprintf(file_name, lineno, func_name, indent(level + 1, "->nvenv is..."),
                     NP_BASE_ADDR(np->nvenv));
            if (ptr_seen(np->nvenv)) {
                _dprintf(file_name, lineno, func_name,
                         indent(level, "WARN: ptr %p already dumped; not following it"), np->nvenv);
            } else {
                _dprint_nvp(file_name, lineno, func_name, level + 1, "->nvenv", np->nvenv);
            }
        }
    }
}

// Diagnostic print a struct Namref object.
void _dprint_nrp(const char *file_name, const int lineno, const char *func_name, int level,
                 const char *var_name, const void *vp) {
    const struct Namref *nr = vp;

    if (level == 0) clear_ptrs();

    _dprintf(file_name, lineno, func_name, indent(level, "struct Namref %s @ %p"), var_name,
             NP_BASE_ADDR(nr));
    if (!nr) return;

    _dprintf(file_name, lineno, func_name, indent(level + 1, "->np is..."));
    if (ptr_seen(nr->np)) {
        _dprintf(file_name, lineno, func_name,
                 indent(level, "WARN: ptr %p already dumped; not following it"), nr->np);
    } else {
        _dprint_nvp(file_name, lineno, func_name, level + 1, "np", nr->np);
    }

    _dprintf(file_name, lineno, func_name, indent(level + 1, "->table is..."));
    if (ptr_seen(nr->table)) {
        _dprintf(file_name, lineno, func_name,
                 indent(level, "WARN: ptr %p already dumped; not following it"), nr->table);
    } else {
        _dprint_nvp(file_name, lineno, func_name, level + 1, "table", nr->table);
    }

    _dprintf(file_name, lineno, func_name, indent(level + 1, "->oldnp is..."));
    if (ptr_seen(nr->oldnp)) {
        _dprintf(file_name, lineno, func_name,
                 indent(level, "WARN: ptr %p already dumped; not following it"), nr->oldnp);
    } else {
        _dprint_nvp(file_name, lineno, func_name, level + 1, "oldnp", nr->oldnp);
    }

    _dprintf(file_name, lineno, func_name, indent(level + 1, "->sub %p |%s|"),
             NP_BASE_ADDR(nr->sub), nr->sub);
}
