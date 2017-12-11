/* : : generated from lib by iffe version 2013-11-14 : : */
#ifndef _def_lib_features
#define _def_lib_features	1

#define _mem_d_reclen_direct    1   /* d_reclen is a member of struct direct */
#define _mem_d_fileno_dirent    1   /* d_fileno is a member of struct dirent */
#define _mem_d_ino_dirent   1   /* d_ino is a member of struct dirent */
#define _mem_d_off_dirent   1   /* d_off is a member of struct dirent */
#define _mem_d_reclen_dirent    1   /* d_reclen is a member of struct dirent */
#define _mem_d_type_dirent  1   /* d_type is a member of struct dirent */
#define _mem_l_type_flock   1   /* l_type is a member of struct flock */


#define _fd_self_dir_fmt    "/proc/self/fd/%d%s%s"
#define _fd_pid_dir_fmt     "/proc/%d/fd/%d%s%s"
#if !_AST_no_spawnveg
#define _use_spawnveg   1
#endif

#endif
