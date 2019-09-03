/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1982-2014 AT&T Intellectual Property          *
 *                      and is licensed under the                       *
 *                 Eclipse Public License, Version 1.0                  *
 *                    by AT&T Intellectual Property                     *
 *                                                                      *
 *                A copy of the License is available at                 *
 *          http://www.eclipse.org/org/documents/epl-v10.html           *
 *         (with md5 checksum b35adb5213ca9657e911e9befb180842)         *
 *                                                                      *
 *              Information and Software Systems Research               *
 *                            AT&T Research                             *
 *                           Florham Park NJ                            *
 *                                                                      *
 *                    David Korn <dgkorn@gmail.com>                     *
 *                                                                      *
 ***********************************************************************/
#include "config_ast.h"  // IWYU pragma: keep

#include <signal.h>
#include <stddef.h>

#include "error.h"
#include "fault.h"
#include "shtable.h"

#if defined(SIGCLD) && !defined(SIGCHLD)
#define SIGCHLD SIGCLD
#endif  // defined(SIGCLD) && !defined(SIGCHLD)

#define VAL(sig, mode) ((sig + 1) | ((mode) << SH_SIGBITS))
#define TRAP(n) (((n) | SH_TRAP) - 1)

#ifndef ERROR_dictionary
#define ERROR_dictionary(s) (s)
#endif  // !ERROR_dictionary
#define S(s) ERROR_dictionary(s)

//
// This is a table that gives numbers and default settings to each signal. The signal numbers go in
// the low bits and the attributes go in the high bits. The names must be ASCII sorted lo-hi.
//
const struct shtable2 shtab_signals[] = {
#ifdef SIGABRT
    {"ABRT", VAL(SIGABRT, SH_SIGDONE), S("Abort"), NULL},
#endif  // SIGABRT
#ifdef SIGAIO
    {"AIO", VAL(SIGAIO, SH_SIGIGNORE), S("Asynchronous I/O"), NULL},
#endif  // SIGAIO
#ifdef SIGALRM
    {"ALRM", VAL(SIGALRM, SH_SIGDONE), S("Alarm call"), NULL},
#endif  // SIGALRM
#ifdef SIGALRM1
    {"ALRM1", VAL(SIGALRM1, SH_SIGDONE), S("Scheduling - reserved"), NULL},
#endif  // SIGALRM1
#ifdef SIGAPOLLO
    {"APOLLO", VAL(SIGAPOLLO, 0), S("SIGAPOLLO"), NULL},
#endif  // SIGAPOLLO
#ifdef SIGBUS
    {"BUS", VAL(SIGBUS, SH_SIGDONE), S("Bus error"), NULL},
#endif  // SIGBUS
#ifdef SIGCANCEL
    {"CANCEL", VAL(SIGCANCEL, SH_SIGIGNORE), S("Thread cancellation"), NULL},
#endif  // SIGCANCEL
#ifdef SIGCHLD
    {"CHLD", VAL(SIGCHLD, SH_SIGFAULT), S("Death of Child"), NULL},
#ifdef SIGCLD
#if SIGCLD != SIGCHLD
    {"CLD", VAL(SIGCLD, SH_SIGFAULT), S("Death of Child"), NULL},
#endif  // SIGCLD != SIGCHLD
#endif  // SIGCLD
#else   // SIGCHLD
#ifdef SIGCLD
    {"CHLD", VAL(SIGCLD, SH_SIGFAULT), S("Death of Child"), NULL},
#endif  // SIGCLD
#endif  // SIGCHLD
#ifdef SIGCONT
    {"CONT", VAL(SIGCONT, SH_SIGIGNORE), S("Stopped process continued"), NULL},
#endif  // SIGCONT
#ifdef SIGCPUFAIL
    {"CPUFAIL", VAL(SIGCPUFAIL, 0), S("Predictive processor deconfiguration"), NULL},
#endif  // SIGRETRACT
    {"DEBUG", VAL(TRAP(SH_DEBUGTRAP), 0), "", NULL},
#ifdef SIGDANGER
    {"DANGER", VAL(SIGDANGER, 0), S("System crash soon"), NULL},
#endif  // SIGDANGER
#ifdef SIGDIL
    {"DIL", VAL(SIGDIL, 0), S("DIL signal"), NULL},
#endif  // SIGDIL
#ifdef SIGEMT
    {"EMT", VAL(SIGEMT, SH_SIGDONE), S("EMT trap"), NULL},
#endif  // SIGEMT
    {"ERR", VAL(TRAP(SH_ERRTRAP), 0), "", NULL},
#ifdef SIGERR
    {"ERR", VAL(SIGERR, 0), "", NULL},
#endif  // SIGERR
    {"EXIT", VAL(0, 0), "", NULL},
    {"FPE", VAL(SIGFPE, SH_SIGDONE), S("Floating exception"), NULL},
#ifdef SIGFREEZE
    {"FREEZE", VAL(SIGFREEZE, SH_SIGIGNORE), S("Special signal used by CPR"), NULL},
#endif  // SIGFREEZE
#ifdef SIGGRANT
    {"GRANT", VAL(SIGGRANT, 0), S("Grant monitor mode"), NULL},
#endif  // SIGGRANT
    {"HUP", VAL(SIGHUP, SH_SIGDONE), S("Hangup"), NULL},
    {"ILL", VAL(SIGILL, SH_SIGDONE), S("Illegal instruction"), NULL},
#ifdef SIGINFO
    {"INFO", VAL(SIGINFO, SH_SIGIGNORE), S("Information request"), NULL},
#endif  // SIGINFO
#ifdef JOBS
    {"INT", VAL(SIGINT, SH_SIGINTERACTIVE), S("Interrupt"), NULL},
#else   // JOBS
    {"INT", VAL(SIGINT, SH_SIGINTERACTIVE), "", NULL},
#endif  // JOBS
#ifdef SIGIO
    {"IO", VAL(SIGIO, SH_SIGDONE), S("IO signal"), NULL},
#endif  // SIGIO
#ifdef SIGIOT
    {"IOT", VAL(SIGIOT, SH_SIGDONE), S("Abort"), NULL},
#endif  // SIGIOT
#ifdef SIGJVM1
    {"JVM1", VAL(SIGJVM1, SH_SIGIGNORE), S("Special signal used by Java Virtual Machine"), NULL},
#endif  // SIGJVM1
#ifdef SIGJVM2
    {"JVM2", VAL(SIGJVM2, SH_SIGIGNORE), S("Special signal used by Java Virtual Machine"), NULL},
#endif  // SIGJVM2
    {"KEYBD", VAL(TRAP(SH_KEYTRAP), 0), "", NULL},
#ifdef SIGKILL
    {"KILL", VAL(SIGKILL, 0), S("Killed"), NULL},
#endif  // SIGKILL
#ifdef SIGLAB
    {"LAB", VAL(SIGLAB, 0), S("Security label changed"), NULL},
#endif  // SIGLAB
#ifdef SIGLOST
    {"LOST", VAL(SIGLOST, SH_SIGDONE), S("Resources lost"), NULL},
#endif  // SIGLOST
#ifdef SIGLWP
    {"LWP", VAL(SIGLWP, SH_SIGIGNORE), S("Special signal used by thread library"), NULL},
#endif  // SIGLWP
#ifdef SIGMIGRATE
    {"MIGRATE", VAL(SIGMIGRATE, 0), S("Migrate process"), NULL},
#endif  // SIGMIGRATE
#ifdef SIGMSG
    {"MSG", VAL(SIGMSG, 0), S("Ring buffer input data"), NULL},
#endif  // SIGMSG
#ifdef SIGPHONE
    {"PHONE", VAL(SIGPHONE, 0), S("Phone interrupt"), NULL},
#endif  // SIGPHONE
#ifdef SIGPIPE
#ifdef JOBS
    {"PIPE", VAL(SIGPIPE, SH_SIGDONE), S("Broken Pipe"), NULL},
#else   // JOBS
    {"PIPE", VAL(SIGPIPE, SH_SIGDONE), "", NULL},
#endif  // JOBS
#endif  // SIGPIPE
#ifdef SIGPOLL
    {"POLL", VAL(SIGPOLL, SH_SIGDONE), S("Polling alarm"), NULL},
#endif  // SIGPOLL
#ifdef SIGPROF
    {"PROF", VAL(SIGPROF, SH_SIGDONE), S("Profiling time alarm"), NULL},
#endif  // SIGPROF
#ifdef SIGPRE
    {"PRE", VAL(SIGPRE, SH_SIGDONE), S("Programming exception"), NULL},
#endif  // SIGPRE
#ifdef SIGPWR
#if SIGPWR > 0
    {"PWR", VAL(SIGPWR, SH_SIGIGNORE), S("Power fail"), NULL},
#endif
#endif  // SIGPWR
#ifdef SIGQUIT
    {"QUIT", VAL(SIGQUIT, SH_SIGDONE | SH_SIGINTERACTIVE), S("Quit"), NULL},
#endif  // SIGQUIT
#ifdef SIGRETRACT
    {"RETRACT", VAL(SIGRETRACT, 0), S("Relinquish monitor mode"), NULL},
#endif  // SIGRETRACT
#ifdef SIGRTMIN
    {"RTMIN", VAL(SH_SIGRTMIN, SH_SIGRUNTIME), S("Lowest priority realtime signal"), NULL},
#endif  // SIGRTMIN
#ifdef SIGRTMAX
    {"RTMAX", VAL(SH_SIGRTMAX, SH_SIGRUNTIME), S("Highest priority realtime signal"), NULL},
#endif  // SIGRTMAX
#ifdef SIGSAK
    {"SAK", VAL(SIGSAK, 0), S("Secure attention key"), NULL},
#endif  // SIGSAK
    {"SEGV", VAL(SIGSEGV, 0), S("Memory fault"), NULL},
#ifdef SIGSOUND
    {"SOUND", VAL(SIGSOUND, 0), S("Sound completed"), NULL},
#endif  // SIGSOUND
#ifdef SIGSTOP
    {"STOP", VAL(SIGSTOP, 0), S("Stopped (SIGSTOP)"), NULL},
#endif  // SIGSTOP
#ifdef SIGSYS
    {"SYS", VAL(SIGSYS, SH_SIGDONE), S("Bad system call"), NULL},
#endif  // SIGSYS
#ifdef SIGSTKFLT
    {"STKFLT", VAL(SIGSTKFLT, SH_SIGDONE), S("Stack Fault"), NULL},
#endif  // SIGSTKFLT
    {"TERM", VAL(SIGTERM, SH_SIGDONE | SH_SIGINTERACTIVE), S("Terminated"), NULL},
#ifdef SIGTHAW
    {"THAW", VAL(SIGTHAW, SH_SIGIGNORE), S("Special signal used by CPR"), NULL},
#endif  // SIGTHAW
#ifdef SIGTINT
#ifdef JOBS
    {"TINT", VAL(SIGTINT, 0), S("Interrupt"), NULL},
#else   // JOBS
    {"TINT", VAL(SIGTINT, 0), "", NULL},
#endif  // JOBS
#endif  // SIGTINT
#ifdef SIGTRAP
    {"TRAP", VAL(SIGTRAP, SH_SIGDONE), S("Trace/BPT trap"), NULL},
#endif  // SIGTRAP
#ifdef SIGTSTP
    {"TSTP", VAL(SIGTSTP, 0), S("Stopped"), NULL},
#endif  // SIGTSTP
#ifdef SIGTTIN
    {"TTIN", VAL(SIGTTIN, 0), S("Stopped (SIGTTIN)"), NULL},
#endif  // SIGTTIN
#ifdef SIGTTOU
    {"TTOU", VAL(SIGTTOU, 0), S("Stopped (SIGTTOU)"), NULL},
#endif  // SIGTTOU
#ifdef SIGURG
    {"URG", VAL(SIGURG, SH_SIGIGNORE), S("Socket interrupt"), NULL},
#endif  // SIGURG
#ifdef SIGUSR1
    {"USR1", VAL(SIGUSR1, SH_SIGDONE), S("User signal 1"), NULL},
#endif  // SIGUSR1
#ifdef SIGUSR2
    {"USR2", VAL(SIGUSR2, SH_SIGDONE), S("User signal 2"), NULL},
#endif  // SIGUSR2
#ifdef SIGVIRT
    {"VIRT", VAL(SIGVIRT, 0), S("Virtual timer alarm"), NULL},
#endif  // SIGVIRT
#ifdef SIGVTALRM
    {"VTALRM", VAL(SIGVTALRM, SH_SIGDONE), S("Virtual time alarm"), NULL},
#endif  // SIGVTALRM
#ifdef SIGWAITING
    {"WAITING", VAL(SIGWAITING, SH_SIGIGNORE), S("All threads blocked"), NULL},
#endif  // SIGWAITING
#ifdef SIGWINCH
    {"WINCH", VAL(SIGWINCH, SH_SIGIGNORE), S("Window size change"), NULL},
#endif  // SIGWINCH
#ifdef SIGXCPU
    {"XCPU", VAL(SIGXCPU, SH_SIGDONE | SH_SIGINTERACTIVE), S("Exceeded CPU time limit"), NULL},
#endif  // SIGXCPU
#ifdef SIGXFSZ
    {"XFSZ", VAL(SIGXFSZ, SH_SIGDONE | SH_SIGINTERACTIVE), S("Exceeded file size limit"), NULL},
#endif  // SIGXFSZ
#ifdef SIGXRES
    {"XRES", VAL(SIGXRES, SH_SIGDONE | SH_SIGINTERACTIVE), S("Exceeded resource control"), NULL},
#endif  // SIGRES
    {"", 0, NULL, NULL}};

const struct shtable4 shtab_siginfo_codes[] = {{SIGCHLD, CLD_EXITED, "EXITED"},
                                               {SIGCHLD, CLD_DUMPED, "DUMPED"},
                                               {SIGCHLD, CLD_KILLED, "KILLED"},
#ifdef CLD_STOPPED
                                               {SIGCHLD, CLD_STOPPED, "STOPPED"},
#endif  // CLD_STOPPED
#ifdef CLD_CONTINUED
                                               {SIGCHLD, CLD_CONTINUED, "CONTINUED"},
#endif  // CLD_CONTINUED
#ifdef CLD_TRAPPED
                                               {SIGCHLD, CLD_TRAPPED, "TRAPPED"},
#endif  // CLD_TRAPPED
#ifdef SIGILL
#ifdef POLL_IN
                                               {SIGILL, POLL_IN, "IN"},
#endif  // POLL_IN
#ifdef ILL_ILLOPC
                                               {SIGILL, ILL_ILLOPC, "ILLOPC"},
#endif  // ILL_ILLOPC
#ifdef ILL_ILLOPN
                                               {SIGILL, ILL_ILLOPN, "ILLOP"},
#endif  // ILL_ILLOPN
#ifdef ILL_ADR
                                               {SIGILL, ILL_ADR, "ADR"},
#endif  // ILL_ADR
#ifdef ILL_TRP
                                               {SIGILL, ILL_TRP, "TRP"},
#endif  // ILL_TRP
#ifdef ILL_PRVOPC
                                               {SIGILL, ILL_PRVOPC, "PRVOPC"},
#endif  // ILL_PRVOPC
#ifdef ILL_COPROC
                                               {SIGILL, ILL_COPROC, "COPROC"},
#endif  // ILL_PRVOPC
#ifdef ILL_BADSTK
                                               {SIGILL, ILL_BADSTK, "BADSTK"},
#endif  // ILL_BADSTK
#endif  // SIGILL
#ifdef SIGPOLL
#ifdef POLL_IN
                                               {SIGPOLL, POLL_IN, "IN"},
#endif  // POLL_IN
#ifdef POLL_OUT
                                               {SIGPOLL, POLL_OUT, "OUT"},
#endif  // POLL_OUT
#ifdef POLL_MSG
                                               {SIGPOLL, POLL_MSG, "MSG"},
#endif  // POLL_MSG
#ifdef POLL_ERR
                                               {SIGPOLL, POLL_ERR, "ERR"},
#endif  // POLL_ERR
#ifdef POLL_PRI
                                               {SIGPOLL, POLL_PRI, "PRI"},
#endif  // POLL_PRI
#ifdef POLL_HUP
                                               {SIGPOLL, POLL_HUP, "HUP"},
#endif  // POLL_HUP
#endif  // SIGPOLL
        //
        // Entries with sig==0 must be at the end of the list to prevent possible clashes with
        // signal-specific codes.
        //

                                               // On some systems, e.g., macOS & BSD, si_code is
                                               // zero rather than whatever SI_USER is defined to
                                               // be when a signal is sent via kill().
                                               {0, 0, "SI_USER"},
#ifdef SI_USER
                                               {0, SI_USER, "SI_USER"},
#endif  // SI_USER
#ifdef SI_QUEUE
                                               {0, SI_QUEUE, "SI_QUEUE"},
#endif  // SI_QUEUE
#ifdef SI_TIMER
                                               {0, SI_TIMER, "SI_TIMER"},
#endif  // SI_TIMER
#ifdef SI_ASYNCIO
                                               {0, SI_ASYNCIO, "SI_ASYNCIO"},
#endif  // SI_ASYNCIO
#ifdef SI_MESGQ
                                               {0, SI_MESGQ, "SI_MESGQ"},
#endif  // SI_MESGQ
#ifdef SI_NOINFO
                                               {0, SI_NOINFO, "SI_NOINFO"},
#endif  // SI_NOINFO
#ifdef SI_DTRACE
                                               {0, SI_DTRACE, "SI_DTRACE"},
#endif  // SI_DTRACE
#ifdef SI_RCTL
                                               {0, SI_RCTL, "SI_RCTL"},
#endif  // SI_RCTL
#ifdef SI_LWP
                                               {0, SI_LWP, "SI_LWP"},
#endif  // SI_LWP
#ifdef SI_KERNEL
                                               {0, SI_KERNEL, "SI_KERNEL"},
#endif  // SI_KERNEL
#ifdef SI_SIGIO
                                               {0, SI_SIGIO, "SI_SIGIO"},
#endif  // SI_SIGIO
#ifdef SI_TKILL
                                               {0, SI_TKILL, "SI_TKILL"},
#endif  // SI_TKILL
#ifdef SI_ASYNCNL
                                               {0, SI_ASYNCNL, "SI_ASYNCNL"},
#endif  // SI_ASYNCNL
                                               {0, 0, NULL}};
