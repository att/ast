// This contains functions useful for debugging ksh code. For example, `_dprint_vtp()` to write
// information about a struct Value to stderr.
//
#include "config_ast.h"  // IWYU pragma: keep

#include <inttypes.h>
#include <stddef.h>
#include <sys/types.h>

#include "ast_assert.h"
#include "name.h"

// This is to give unit tests some control over the addresses displayed. Setting this to the
// address of the first var in the unit test module will cause all pointers to be displayed as
// offsets to that var.
void *_dprint_vt_base_addr = NULL;
#define BASE_ADDR(p) (char *)((char *)(p) - (char *)_dprint_vt_base_addr)

typedef void(vtp_dprintf)(const char *, int, const char *, const struct Value *);

static_fn void _dprint_VT_do_not_use(const char *fname, int lineno, const char *funcname,
                                     const struct Value *vtp) {
    UNUSED(vtp);
    _dprintf(fname, lineno, funcname, "value is undefined since the type is VT_do_not_use");
}

static_fn void _dprint_VT_vp(const char *fname, int lineno, const char *funcname,
                             const struct Value *vtp) {
    _dprintf(fname, lineno, funcname, "value is void* %p", BASE_ADDR(FETCH_VTP(vtp, vp)));
}

static_fn void _dprint_VT_cp(const char *fname, int lineno, const char *funcname,
                             const struct Value *vtp) {
    _dprintf(fname, lineno, funcname, "value is char* %p |%s|", BASE_ADDR(FETCH_VTP(vtp, cp)),
             FETCH_VTP(vtp, cp));
}

static_fn void _dprint_VT_const_cp(const char *fname, int lineno, const char *funcname,
                                   const struct Value *vtp) {
    _dprintf(fname, lineno, funcname, "value is char* %p |%s|", BASE_ADDR(FETCH_VTP(vtp, const_cp)),
             FETCH_VTP(vtp, const_cp));
}

static_fn void _dprint_VT_i(const char *fname, int lineno, const char *funcname,
                            const struct Value *vtp) {
    _dprintf(fname, lineno, funcname, "value is int %i (0x%X)", FETCH_VTP(vtp, i),
             FETCH_VTP(vtp, i));
}

static_fn void _dprint_VT_i16(const char *fname, int lineno, const char *funcname,
                              const struct Value *vtp) {
    _dprintf(fname, lineno, funcname, "value is int16_t %" PRIi16 "(0x%" PRIX16 ")",
             FETCH_VTP(vtp, i16), FETCH_VTP(vtp, i16));
}

static_fn void _dprint_VT_ip(const char *fname, int lineno, const char *funcname,
                             const struct Value *vtp) {
    _dprintf(fname, lineno, funcname, "value is int* %p => %i (0x%X)",
             BASE_ADDR(FETCH_VTP(vtp, ip)), *FETCH_VTP(vtp, ip), *FETCH_VTP(vtp, ip));
}

static_fn void _dprint_VT_i16p(const char *fname, int lineno, const char *funcname,
                               const struct Value *vtp) {
    _dprintf(fname, lineno, funcname, "value is int16_t* %p => %" PRIi16 " (0x%" PRIX16 ")",
             BASE_ADDR(FETCH_VTP(vtp, i16p)), *FETCH_VTP(vtp, i16p), *FETCH_VTP(vtp, i16p));
}

static_fn void _dprint_VT_i32p(const char *fname, int lineno, const char *funcname,
                               const struct Value *vtp) {
    _dprintf(fname, lineno, funcname, "value is int32_t* %p => %" PRIi32 " (0x%" PRIX32 ")",
             BASE_ADDR(FETCH_VTP(vtp, i32p)), *FETCH_VTP(vtp, i32p), *FETCH_VTP(vtp, i32p));
}

static_fn void _dprint_VT_i64p(const char *fname, int lineno, const char *funcname,
                               const struct Value *vtp) {
    _dprintf(fname, lineno, funcname, "value is int64_t* %p => %" PRIi64 " (0x%" PRIX64 ")",
             BASE_ADDR(FETCH_VTP(vtp, i64p)), *FETCH_VTP(vtp, i64p), *FETCH_VTP(vtp, i64p));
}

static_fn void _dprint_VT_dp(const char *fname, int lineno, const char *funcname,
                             const struct Value *vtp) {
    _dprintf(fname, lineno, funcname, "value is double* %p => %g", BASE_ADDR(FETCH_VTP(vtp, dp)),
             *FETCH_VTP(vtp, dp));
}

static_fn void _dprint_VT_fp(const char *fname, int lineno, const char *funcname,
                             const struct Value *vtp) {
    _dprintf(fname, lineno, funcname, "value is float* %p => %g", BASE_ADDR(FETCH_VTP(vtp, fp)),
             *FETCH_VTP(vtp, fp));
}

static_fn void _dprint_VT_sfdoublep(const char *fname, int lineno, const char *funcname,
                                    const struct Value *vtp) {
    _dprintf(fname, lineno, funcname, "value is Sfdouble_t* %p => %Lg",
             BASE_ADDR(FETCH_VTP(vtp, sfdoublep)), *FETCH_VTP(vtp, sfdoublep));
}

static_fn void _dprint_VT_np(const char *fname, int lineno, const char *funcname,
                             const struct Value *vtp) {
    // TODO: Use a function to dump a Namval_t when it becomes available.
    _dprintf(fname, lineno, funcname, "value is Namval_t* %p", BASE_ADDR(FETCH_VTP(vtp, np)));
}

static_fn void _dprint_VT_up(const char *fname, int lineno, const char *funcname,
                             const struct Value *vtp) {
    _dprintf(fname, lineno, funcname,
             "value is struct Value* %p which has this value:", BASE_ADDR(FETCH_VTP(vtp, up)));
    _dprint_vtp(fname, lineno, funcname, FETCH_VTP(vtp, up));
}

static_fn void _dprint_VT_rp(const char *fname, int lineno, const char *funcname,
                             const struct Value *vtp) {
    _dprintf(fname, lineno, funcname, "value is struct Ufunction* %p",
             BASE_ADDR(FETCH_VTP(vtp, rp)));
}

static_fn void _dprint_VT_funp(const char *fname, int lineno, const char *funcname,
                               const struct Value *vtp) {
    _dprintf(fname, lineno, funcname, "value is struct Namfun* %p",
             BASE_ADDR(FETCH_VTP(vtp, funp)));
}

static_fn void _dprint_VT_nrp(const char *fname, int lineno, const char *funcname,
                              const struct Value *vtp) {
    _dprintf(fname, lineno, funcname, "value is struct Namref* %p", BASE_ADDR(FETCH_VTP(vtp, nrp)));
}

static_fn void _dprint_VT_shbltinp(const char *fname, int lineno, const char *funcname,
                                   const struct Value *vtp) {
    _dprintf(fname, lineno, funcname, "value is struct Shbltin_f %p",
             BASE_ADDR(FETCH_VTP(vtp, shbltinp)));
}

static_fn void _dprint_VT_pathcomp(const char *fname, int lineno, const char *funcname,
                                   const struct Value *vtp) {
    _dprintf(fname, lineno, funcname, "value is struct pathcomp* %p",
             BASE_ADDR(FETCH_VTP(vtp, pathcomp)));
}

static_fn void _dprint_VT_pidp(const char *fname, int lineno, const char *funcname,
                               const struct Value *vtp) {
    _dprintf(fname, lineno, funcname, "value is pid_t* %p => %" PRIu64,
             BASE_ADDR(FETCH_VTP(vtp, pidp)), (uint64_t)*FETCH_VTP(vtp, pidp));
}

static_fn void _dprint_VT_uidp(const char *fname, int lineno, const char *funcname,
                               const struct Value *vtp) {
    _dprintf(fname, lineno, funcname, "value is uid_t* %p => %" PRIu64,
             BASE_ADDR(FETCH_VTP(vtp, uidp)), (uint64_t)*FETCH_VTP(vtp, uidp));
}

static_fn void _dprint_VT_dummy(const char *fname, int lineno, const char *funcname,
                                const struct Value *vtp) {
    UNUSED(fname);
    UNUSED(lineno);
    UNUSED(funcname);
    UNUSED(vtp);
}

// This must be kept in the same order as the `enum value_type` in "name.h".
static vtp_dprintf *dprint_vtp_dispatch[] = {
    _dprint_VT_do_not_use, _dprint_VT_vp,   _dprint_VT_cp,       _dprint_VT_const_cp,
    _dprint_VT_i,          _dprint_VT_i16,  _dprint_VT_ip,       _dprint_VT_i16p,
    _dprint_VT_i32p,       _dprint_VT_i64p, _dprint_VT_dp,       _dprint_VT_fp,
    _dprint_VT_sfdoublep,  _dprint_VT_np,   _dprint_VT_up,       _dprint_VT_rp,
    _dprint_VT_funp,       _dprint_VT_nrp,  _dprint_VT_shbltinp, _dprint_VT_pathcomp,
    _dprint_VT_pidp,       _dprint_VT_uidp, _dprint_VT_dummy  // must be last
};

// Diagnostic print a struct Value object.
void _dprint_vtp(const char *fname, int const lineno, const char *funcname, const void *vp) {
    const struct Value *vtp = vp;

    // We do this rather than a sizeof(dprint_vtp_dispatch) check because the latter is a constant
    // expression that causes lint warnings.
    assert(dprint_vtp_dispatch[VT_sentinal] == _dprint_VT_dummy);
    assert(vtp->type >= VT_do_not_use && vtp->type < VT_sentinal);

    _dprintf(fname, lineno, funcname, "type \"%s\" stored @ %s:%d in %s()",
             value_type_names[vtp->type], vtp->filename ? vtp->filename : "undef", vtp->line_num,
             vtp->funcname ? vtp->funcname : "undef");

    (dprint_vtp_dispatch[vtp->type])(fname, lineno, funcname, vtp);
}

// If a unit test has set a base addr for struct Value objects just use a constant for the printed
// address of a Namval_t*. This is needed because different environments have different padding and
// alignment of these structures. That makes it impossible to emit a consistent address in the
// diagnostic message.
#define NP_BASE_ADDR(np) (_dprint_vt_base_addr ? (void *)0x88 : (void *)np)

// Diagnostic print a struct Namval (aka Namval_t) object.
void _dprint_nvp(const char *fname, const int lineno, const char *funcname, const void *vp) {
    const Namval_t *np = vp;

    _dprintf(fname, lineno, funcname, "np %p ->nvname |%s|  ->nvsize %d", NP_BASE_ADDR(np),
             np->nvname, np->nvsize);
    _dprintf(fname, lineno, funcname, "np %p ->nvalue is:", NP_BASE_ADDR(np));
    _dprint_vtp(fname, lineno, funcname, &np->nvalue);
    if (np->nvenv) {
        _dprintf(fname, lineno, funcname, "np %p ->nvenv is %p:", NP_BASE_ADDR(np),
                 NP_BASE_ADDR(np->nvenv));
        _dprint_nvp(fname, lineno, funcname, np->nvenv);
    }
}
