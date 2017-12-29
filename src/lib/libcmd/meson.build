libcmd_files = ['context.c', 'cmdinit.c']
incdir = include_directories('../libast/include', '../libast/features/')

libcmd = library('cmd', libcmd_files,
                include_directories: incdir,
                link_with: libast,
                install: true)
