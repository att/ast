#ifndef _AST_SPAWNVEX_H
#define _AST_SPAWNVEX_H 1

#if USE_SPAWN

#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <locale.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

union _Spawnvex_u;
typedef union _Spawnvex_u Spawnvex_u;

typedef struct Spawnvex_s {
    unsigned int cur;
    int io;
    unsigned int max;
    unsigned int set;
    unsigned int flags;
    unsigned int frame;
    pid_t pgrp;
    int debug;
    int noexec;
    Spawnvex_u *op;
} Spawnvex_t;

typedef struct Spawnvex_noexec_s {
    Spawnvex_t *vex;    /* caller vex handle            */
    void *handle;       /* caller callback handle       */
    const char *path;   /* spawnvex() path              */
    char *const *argv;  /* spawnvex() argv              */
    char *const *envv;  /* spawnvex() envv              */
    unsigned int flags; /* SPAWN_EXEC                   */
    int msgfd;          /* if no return and >= 0 close  */
} Spawnvex_noexec_t;

typedef int (*Spawnvex_f)(void *, uint64_t, uint64_t);

// #define SPAWN_BACKGROUND 0x00001
#define SPAWN_CLEANUP 0x00002
// #define SPAWN_DAEMON 0x00004
#define SPAWN_DEBUG 0x00008
#define SPAWN_EXEC 0x00010
#define SPAWN_FLUSH 0x00020
#define SPAWN_FOREGROUND 0x00040
#define SPAWN_FRAME 0x00100
#define SPAWN_NOCALL 0x00200
// #define SPAWN_ORPHAN 0x00400
// #define SPAWN_OVERLAY 0x00800
// #define SPAWN_READ 0x01000
#define SPAWN_RESET 0x02000
#define SPAWN_UNDO 0x08000
// #define SPAWN_WRITE 0x10000
// #define SPAWN_ZOMBIE 0x20000

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
extern int spawnvex_add(Spawnvex_t *, int64_t, int64_t, Spawnvex_f, void *);
extern int spawnvex_apply(Spawnvex_t *, int, int);
extern int64_t spawnvex_get(Spawnvex_t *, int, int);
extern int spawnvex_close(Spawnvex_t *);

#endif  // USE_SPAWN

#endif
