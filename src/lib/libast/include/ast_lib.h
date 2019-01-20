/* : : generated from lib by iffe version 2013-11-14 : : */
#ifndef lib_features_defined
#define lib_features_defined 1

#define _mem_d_reclen_direct 1 /* d_reclen is a member of struct direct */
#define _mem_d_fileno_dirent 1 /* d_fileno is a member of struct dirent */
#define _mem_d_ino_dirent 1    /* d_ino is a member of struct dirent */
#define _mem_d_off_dirent 1    /* d_off is a member of struct dirent */
#define _mem_d_reclen_dirent 1 /* d_reclen is a member of struct dirent */
#define _mem_d_type_dirent 1   /* d_type is a member of struct dirent */
#define _mem_l_type_flock 1    /* l_type is a member of struct flock */

#define _fd_self_dir_fmt "/proc/self/fd/%d%s%s"
#define _fd_pid_dir_fmt "/proc/%d/fd/%d%s%s"

// https://github.com/att/ast/issues/370
#define _UNIV_DEFAULT "ucb"
#endif
