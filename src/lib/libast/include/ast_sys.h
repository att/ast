/* : : generated from sys by iffe version 2013-11-14 : : */
#ifndef _AST_SYS_H
#define _AST_SYS_H 1
#if __mips == 2 && !defined(_NO_LARGEFILE64_SOURCE)
#define _NO_LARGEFILE64_SOURCE 1
#endif
#if !defined(_NO_LARGEFILE64_SOURCE)
#if !defined(_LARGEFILE64_SOURCE)
#define _LARGEFILE64_SOURCE 1
#endif
#if !defined(_LARGEFILE_SOURCE)
#define _LARGEFILE_SOURCE 1
#endif
#if !defined(_LARGE_FILE_API)
#define _LARGE_FILE_API 1
#endif
#else
#undef _LARGEFILE64_SOURCE
#undef _LARGEFILE_SOURCE
#undef _LARGE_FILE_API
#undef _typ_ino64_t
#undef _typ_struct_dirent64
#undef _lib_creat64
#undef _lib_fstat64
#undef _lib_fstatvfs64
#undef _lib_ftruncate64
#undef _lib_lstat64
#undef _lib_mmap64
#undef _lib_open64
#undef _lib_opendir64
#undef _lib_rewinddir64
#undef _lib_seekdir64
#undef _lib_telldir64
#undef _lib_closedir64
#undef _lib_statvfs64
#undef _lib_truncate64
#endif
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <locale.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#if !_AST_no_spawnveg
typedef struct Spawnvex_s {
    unsigned int cur;
    int io;
#ifdef _SPAWNVEX_PRIVATE_
    _SPAWNVEX_PRIVATE_
#endif
} Spawnvex_t;

typedef struct Spawnvex_noexec_s {
    Spawnvex_t *vex;    /* caller vex handle		*/
    void *handle;       /* caller callback handle	*/
    const char *path;   /* spawnvex() path		*/
    char *const *argv;  /* spawnvex() argv		*/
    char *const *envv;  /* spawnvex() envv		*/
    unsigned int flags; /* SPAWN_EXEC             	*/
    int msgfd;          /* if no return and >= 0 close	*/
} Spawnvex_noexec_t;

typedef int (*Spawnvex_f)(void *, uintmax_t, uintmax_t);

#define SPAWN_BACKGROUND 0x00001
#define SPAWN_CLEANUP 0x00002
#define SPAWN_DAEMON 0x00004
#define SPAWN_DEBUG 0x00008
#define SPAWN_EXEC 0x00010
#define SPAWN_FLUSH 0x00020
#define SPAWN_FOREGROUND 0x00040
#define SPAWN_FRAME 0x00100
#define SPAWN_NOCALL 0x00200
#define SPAWN_ORPHAN 0x00400
#define SPAWN_OVERLAY 0x00800
#define SPAWN_READ 0x01000
#define SPAWN_RESET 0x02000
#define SPAWN_UNDO 0x08000
#define SPAWN_WRITE 0x10000
#define SPAWN_ZOMBIE 0x20000

#define SPAWN_noop (-1)

#define SPAWN_cwd (-2)
#define SPAWN_frame (-3)
#define SPAWN_noexec (-4)
#define SPAWN_pgrp (-5)
#define SPAWN_resetids (-6)
#define SPAWN_sid (-7)
#define SPAWN_sigdef (-8)
#define SPAWN_sigmask (-9)
#define SPAWN_truncate (-10)
#define SPAWN_umask (-11)

extern pid_t spawnveg(const char *, char *const[], char *const[], pid_t);
extern pid_t spawnvex(const char *, char *const[], char *const[], Spawnvex_t *);
extern Spawnvex_t *spawnvex_open(unsigned int);
extern int spawnvex_add(Spawnvex_t *, intmax_t, intmax_t, Spawnvex_f, void *);
extern int spawnvex_apply(Spawnvex_t *, int, int);
extern intmax_t spawnvex_get(Spawnvex_t *, int, int);
extern int spawnvex_close(Spawnvex_t *);

#endif

#if _BLD_ast && defined(__EXPORT__)
#define extern __EXPORT__
#endif
extern int eaccess(const char *, int);
extern int execvpe(const char *, char *const[], char *const[]);
extern char *fgetcwd(int, char *, size_t);
extern char *gettxt(const char *, const char *);
extern void *memalign(size_t, size_t);
extern void *pvalloc(size_t);
extern char *resolvepath(const char *, char *, size_t);
extern size_t strlcat(char *, const char *, size_t);
extern size_t strlcpy(char *, const char *, size_t);
extern void swab(const void *, void *, ssize_t);
#undef extern
#include <stdarg.h>
#include <string.h>
#endif
