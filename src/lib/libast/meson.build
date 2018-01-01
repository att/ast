libast_files = ['features/conftab.c']
incdir = include_directories('include', 'features', 'aso', 'cdt', 'comp',
                             'sfio', 'path', 'port', 'string', 'misc',
                             'vmalloc', 'tm')
libast_c_args = ['-D_BLD_ast', '-D_API_ast=20130625']
cc = meson.get_compiler('c')

syslocale_result = run_command('features/syslocale.sh')

if syslocale_result.returncode() == 0
    syslocale_output = syslocale_result.stdout()
    libast_c_args += [ '-D_locale_utf8_str=' + syslocale_output ]
endif

libpath_result = run_command('features/libpath.sh')

if libpath_result.returncode() == 0
    libpath_output = libpath_result.stdout()
    libast_c_args += [ '-DCONF_LIBPATH=' + libpath_output ]
endif

prog_result = run_command('features/prog.sh')

if prog_result.returncode() == 0
    prog_output = prog_result.stdout()
    libast_c_args += [ '-D_PROC_PROG=' + prog_output ]
endif

# TODO: Check if this test can be trimmed down in size
align_bound_code = '''
#include <stdio.h>
#include <setjmp.h>
#include <inttypes.h>

union _u_
{
	long			u1;
	char*			u2;
	double			u3;
	char			u4[1024];
	intmax_t		u5;
	uintmax_t		u6;
	long double		u7;
	void*			u8;
	char*			(*u9)();
	jmp_buf			u10;
};

struct _s_
{
	char		s1;
	union _u_	s2;
};

#define roundof(x,y)	(((x)+((y)-1))&~((y)-1))

static union _u_	u;
static union _u_	v;

int
main()
{
	int	i;
	int	j;
	int	k;

	int		align0;
	int		align1;
	int		align2;
	unsigned long	bit1;
	unsigned long	bit2;
	unsigned long	bits0;
	unsigned long	bits1;
	unsigned long	bits2;

	u.u2 = u.u4;
	v.u2 = u.u2 + 1;
	bit1 = u.u1 ^ v.u1;
	v.u2 = u.u2 + 2;
	bit2 = u.u1 ^ v.u1;
	align0 = sizeof(struct _s_) - sizeof(union _u_);
	bits0 = 0;
	k = 0;
	for (j = 0; j < align0; j++)
	{
		u.u2 = u.u4 + j;
		bits1 = 0;
		for (i = 0; i < align0; i++)
		{
			v.u2 = u.u2 + i;
			bits1 |= u.u1 ^ v.u1;
		}
		if (!bits0 || bits1 < bits0)
		{
			bits0 = bits1;
			k = j;
		}
	}
	align1 = roundof(align0, 2);
	u.u2 = u.u4 + k;
	for (bits1 = bits0; i < align1; i++)
	{
		v.u2 = u.u2 + i;
		bits1 |= u.u1 ^ v.u1;
	}
	align2 = roundof(align0, 4);
	for (bits2 = bits1; i < align2; i++)
	{
		v.u2 = u.u2 + i;
		bits2 |= u.u1 ^ v.u1;
	}
#if _X86_ || _X64_
	#if _X64
            printf("ALIGN_BOUND2=16 ");
	#else
            printf("ALIGN_BOUND2=8 ");
	#endif
#else
	printf("ALIGN_BOUND2=%d ", align2);
#endif
	if (align1 == align2)
	{
		printf("ALIGN_BOUND1=ALIGN_BOUND2 ");
	}
	else
	{
		printf("ALIGN_BOUND1=%d ", align1);
	}

    if (align0 == align2)
	{
		printf("ALIGN_BOUND=ALIGN_BOUND2");
	}
	else if (align0 == align1)
	{
		printf("ALIGN_BOUND=ALIGN_BOUND1");
	}
	else
	{
		printf("ALIGN_BOUND=1");
	}
    return 0;
}
'''

align_bound_result = cc.run(align_bound_code, name: 'align bound check')

if align_bound_result.returncode() == 0
    align_bound_result_split = align_bound_result.stdout().split(' ')
    # Meson escapes strings strangely and I could not get my head around it
    # Ideally we should be able to print macro defintions in the test and just
    # use them here. This loop should be unnecessary.
    foreach element : align_bound_result_split
        element_split = element.split('=')
        name = element_split[0]
        value = element_split[1]
        if (name == 'ALIGN_BOUND')
            libast_c_args += [ '-DALIGN_BOUND=' + value ]
        elif (name == 'ALIGN_BOUND1')
            libast_c_args += [ '-DALIGN_BOUND1=' + value ]
        elif (name == 'ALIGN_BOUND2')
            libast_c_args += [ '-DALIGN_BOUND2=' + value ]
        endif
    endforeach
endif

if not cc.has_function('isnanl', prefix: '#include <math.h>')
    libast_c_args += [ '-Disnanl=isnan' ]
endif

if cc.has_function('utimensat', prefix: '#include <sys/stat.h>')
    libast_c_args += [ '-D_lib_utimensat' ]
elif cc.has_function('utimets', prefix: '#include <sys/stat.h>')
    libast_c_args += [ '-D_lib_utimets' ]
endif

if cc.has_function('sysinfo', prefix: '#include <sys/sysinfo.h>')
    libast_c_args += [ '-D_lib_sysinfo=1' ]
endif

if cc.has_function('pipe2', prefix: '#include <unistd.h>')
    libast_c_args += [ '-D_lib_pipe2=1' ]
endif

if cc.has_member('struct stat','st_mtim', prefix: '#include <sys/stat.h>')
    libast_c_args += [ '-DSTAT_ST_MTIM=1' ]
elif cc.has_member('struct stat', 'st_mtimespec', prefix: '#include <sys/stat.h>')
    # Under macOS modification time field is defined as mtimespec
    libast_c_args += [ '-DSTAT_ST_MTIMESPEC=1' ]
else
    error('Can not determine modification time field in struct stat')
endif

if cc.has_header('mach-o/dyld.h')
    libast_c_args += [ '-D_hdr_mach_o_dyld']
endif

libast_c_args += [ '-D_ast_sizeof_int=@0@'.format(cc.sizeof('int')) ]
libast_c_args += [ '-D_ast_sizeof_long=@0@'.format(cc.sizeof('long')) ]
libast_c_args += [ '-D_ast_sizeof_size_t=@0@'.format(cc.sizeof('size_t')) ]
libast_c_args += [ '-D_ast_sizeof_off_t=@0@'.format(cc.sizeof('off_t')) ]
libast_c_args += [ '-D_ast_sizeof_pointer=@0@'.format(cc.sizeof('void*')) ]

subdir('aso')
subdir('comp')
subdir('cdt')
subdir('disc')
subdir('hash')
subdir('sfio')
subdir('path')
subdir('port')
subdir('regex')
subdir('string')
subdir('misc')
subdir('tm')
subdir('vmalloc')

m_dep = cc.find_library('m', required: false)
iconv_dep = cc.find_library('iconv', required: false)

libast = library('ast', libast_files,
                 include_directories: incdir,
                 c_args: libast_c_args,
                 dependencies: [m_dep, iconv_dep],
                 install: true)
