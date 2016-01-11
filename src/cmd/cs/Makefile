:PACKAGE: ast

LICENSE = since=1990,author=gsf

DEBUG ==

cs :: RELEASE cs.c -lcs

css :: css.c -lcs

cat tcp :SERVICE: cat.c

dbm tcp :SERVICE: dbm.c -ldbm

fs_env tcp :SERVICE: fs_env.c

mbb tcp :SERVICE: mbb.c

nam fdp :SERVICE: nam.c

pid udp :SERVICE: pid.c

rsh tcp :SERVICE: rsh.c

tst-old tcp :SERVICE: CCFLAGS=-g tst-old.c

tst tcp :SERVICE: CCFLAGS=-g tst.c

6263 tcp :SERVICE: 6263.c

.SOURCE : vcs_src

imount :: mnt_imount.h mnt_imount.c mnt_list.c mnt_mount.c mnt_mtab.c \
	mnt_touch.c mnt_umount.c mnt_vcs.c \
	-lcs

vcs tcp :SERVICE: vcs.c ifs_agent.c ifs_network.c ifs_func.c \
        ifs_ftp.c ifs_gopher.c ifs_http.c ifs_news.c ifs_rsh.c \
	vcs_rscs.c vcs_checkout.c vcs_attr.c vcs_dir.c vcs_tag.c \
	vcs_misc.c vcs_message.c vcs_search.c vcs_marker.c vcs_replica.c \
	-lodelta

-ldbm : .DONTCARE
