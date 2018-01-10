/* : : generated from shellapi by iffe version 2013-11-14 : : */
#ifndef _SHELL_API_H
#define _SHELL_API_H 1
#define _sys_types 1 /* #include <sys/types.h> ok */
#define SHELLAPI(rel) (_BLD_shell || !_API_shell || _API_shell >= rel)

#ifndef _SHELL_API_IMPLEMENT
#define _SHELL_API_IMPLEMENT 1

#if !defined(_API_shell) && defined(_API_DEFAULT)
#define _API_shell _API_DEFAULT
#endif

#if SHELLAPI(20120720)
#undef sh_addbuiltin
#define sh_addbuiltin sh_addbuiltin_20120720
#endif

#if SHELLAPI(20120720)
#undef sh_bltin_tree
#define sh_bltin_tree sh_bltin_tree_20120720
#endif

#if SHELLAPI(20120720)
#undef sh_eval
#define sh_eval sh_eval_20120720
#endif

#if SHELLAPI(20120720)
#undef sh_exit
#define sh_exit sh_exit_20120720
#endif

#if SHELLAPI(20120720)
#undef sh_fd2sfio
#define sh_fd2sfio sh_fd2sfio_20120720
#endif

#if SHELLAPI(20120720)
#undef sh_fun
#define sh_fun sh_fun_20120720
#endif

#if SHELLAPI(20120720)
#undef sh_funscope
#define sh_funscope sh_funscope_20120720
#endif

#if SHELLAPI(20120720)
#undef sh_getscope
#define sh_getscope sh_getscope_20120720
#endif

#if SHELLAPI(20120720)
#undef sh_isoption
#define sh_isoption sh_isoption_20120720
#endif

#if SHELLAPI(20120720)
#undef sh_menu
#define sh_menu sh_menu_20120720
#endif

#if SHELLAPI(20120720)
#undef sh_offoption
#define sh_offoption sh_offoption_20120720
#endif

#if SHELLAPI(20120720)
#undef sh_onoption
#define sh_onoption sh_onoption_20120720
#endif

#if SHELLAPI(20120720)
#undef sh_pathopen
#define sh_pathopen sh_pathopen_20120720
#endif

#if SHELLAPI(20120720)
#undef sh_reinit
#define sh_reinit sh_reinit_20120720
#endif

#if SHELLAPI(20120720)
#undef sh_run
#define sh_run sh_run_20120720
#endif

#if SHELLAPI(20120720)
#undef sh_setscope
#define sh_setscope sh_setscope_20120720
#endif

#if SHELLAPI(20120720)
#undef sh_strnum
#define sh_strnum sh_strnum_20120720
#endif

#if SHELLAPI(20120720)
#undef sh_trap
#define sh_trap sh_trap_20120720
#endif

#define _API_shell_MAP                                                                  \
    "sh_addbuiltin_20120720 sh_bltin_tree_20120720 sh_eval_20120720 sh_exit_20120720 "  \
    "sh_fd2sfio_20120720 sh_fun_20120720 sh_funscope_20120720 sh_getscope_20120720 "    \
    "sh_isoption_20120720 sh_menu_20120720 sh_offoption_20120720 sh_onoption_20120720 " \
    "sh_pathopen_20120720 sh_reinit_20120720 sh_run_20120720 sh_setscope_20120720 "     \
    "sh_strnum_20120720 sh_trap_20120720"

#endif

#endif
