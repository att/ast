//
// This module contains fallback implementations of functions used elsewhere in the code.
// Each fallback implementation is only enabled if the system doesn't provide it.
//
#include "config_ast.h"  // IWYU pragma: keep

// We keep all these includes because it's simpler and cleaner to do this than wrap them in
// `#if !_lib_mkostemp` type pragmas.
#include <errno.h>     // IWYU pragma: keep
#include <fcntl.h>     // IWYU pragma: keep
#include <stdio.h>     // IWYU pragma: keep
#include <stdlib.h>    // IWYU pragma: keep
#include <string.h>    // IWYU pragma: keep
#include <sys/stat.h>  // IWYU pragma: keep
#include <unistd.h>    // IWYU pragma: keep

#include "ast.h"         // IWYU pragma: keep
#include "ast_assert.h"  // IWYU pragma: keep

//
// We define this symbol, which is otherwise unused, to ensure this module isn't empty. That's
// because empty modules can cause build time warnings.
//
int __do_not_use_this_fallback_sym = 0;

#if !_lib_eaccess
#if _lib_euidaccess
// System doesn't have eaccess() but does have the equivalent euidaccess() so use that.
int eaccess(const char *pathname, int mode) { return euidaccess(pathname, mode); }
#elif _lib_faccessat
// System doesn't have eaccess() but does have faccessat() so use that.
int eaccess(const char *pathname, int mode) {
    return faccessat(AT_FDCWD, pathname, mode, AT_EACCESS);
}
#else
// The platform doesn't have eaccess(), euidaccess(), or faccessat() so we have to roll our own.
// This may not be optimal in that it calls access() when it might be possible to return an answer
// based on what we already know. Since this is a fallback function we hope not to use we're
// striving for simplicity and clarity.
int eaccess(const char *pathname, int mode) {
    // In case the user passed bogus bits for the mode tell them we can't determine what they want.
    if (mode & ~(F_OK | X_OK | W_OK | R_OK)) {
        errno = EINVAL;
        return -1;
    }

    uid_t uid = getuid();
    gid_t gid = getgid();
    uid_t euid = geteuid();
    gid_t egid = getegid();

    // If we aren't suid or sgid then we can just use access().
    // Similarly if we're just checking if the file exists just use access().
    if ((euid == uid && egid == gid) || mode == F_OK) return access(pathname, mode);

    struct stat file_status;
    if (stat(pathname, &file_status) != 0) return -1;

    if (euid == 0) {
        // The root (super) user can read/write any file that exists.
        if ((mode & X_OK) == 0) return 0;
        // The root (super) user can execute any file with execute permission.
        if (file_status.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) return 0;
    }

    // The bit shifts below rely on the common UNIX convention for encoding file permissions as
    // three groups (ugo) of three bits (rwx). If we ever run on a platform with a different scheme
    // (unlikely but not impossible) we'll have to do these tests in a less elegant manner.
    if (euid == file_status.st_uid) {
        if ((file_status.st_mode & (mode << 6)) == (mode << 6)) return 0;
    }

    if (egid == file_status.st_gid) {
        if ((file_status.st_mode & (mode << 3)) == (mode << 3)) return 0;
    }

    // Our euid and egid did not match the file so just fall back to access().
    return access(pathname, mode);
}
#endif  // _lib_faccessat
#endif  // !_lib_eaccess

#if !_lib_mkostemp
// This is a fallback in case the system doesn't provide it.
int mkostemp(char *template, int oflags) {
    for (int i = 10; i; i--) {
#ifndef __clang_analyzer__
        // cppcheck-suppress mktempCalled
        char *tp = mktemp(template);
        assert(tp);
#endif

        int fd = open(template, O_CREAT | O_RDWR | O_EXCL | oflags, S_IRUSR | S_IWUSR);
        if (fd != -1) return fd;
    }
    return -1;
}
#endif  // !_lib_mkostemp

#if _lib_lchmod_fchmodat_fallback
// This fallback is for platforms like OpenBSD which don't have lchmod() but provide the means for
// implementing it via fchmodat().
int lchmod(const char *path, mode_t mode) {
    return fchmodat(AT_FDCWD, path, mode, AT_SYMLINK_NOFOLLOW);
}
#endif
