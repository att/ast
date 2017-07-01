/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1987-2012 AT&T Intellectual Property          *
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
*               Glenn Fowler <glenn.s.fowler@gmail.com>                *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * pax option table
 */

#include "pax.h"

Option_t		options[] =
{
{
	0,
},
{
	"action",
	'A',
	OPT_action,
	"Input/output file path match and filter command. \acommand\a\
	is applied to each file that matches \apattern\a as it is read\
	from or written to the archive. \bX\b is any delimiter not occurring\
	in \apattern\a. \acommand\a is split into space separated arguments,\
	and is executed with the pathname of the file to be processed as the\
	last argument. The standard output of the resulting command is read\
	by \bpax\b.",
	"\bX\bpattern\bX\bcommand",
},
{
	"append",
	'a',
	OPT_append,
	"Append to end of archive.",
},
{
	"atime",
	0,
	OPT_atime,
	"Preserve or set file access times.",
	"time",
	0,
	OPT_HEADER|OPT_OPTIONAL,
	4
},
{
	"base",
	'z',
	OPT_base,
	"Two archives are required for delta operations. \b--file\b names\
	the delta archive and \b--base\b names the delta base archive.\
	If \aarchive\a is \b-\b then the base is ignored (the actual delta\
	sizes are reported and restored) on input and the delta is\
	compressed on output.",
	"archive",
},
{
	"blocksize",
	'b',
	OPT_blocksize,
	"Input/output block size. The default is format specific.",
	"size",
	0,
	OPT_NUMBER,
},
{
	"blok",
	0,
	OPT_blok,
	"Input/output BLOK format for tapes on file.",
	"i|o",
	0,
	OPT_OPTIONAL,
},
{
	"charset",
	0,
	OPT_charset,
	"Data character set name. The character set names are:",
	"charset",
	"\fcharsets\f",
	0,
	OPT_HEADER,
},
{
	"checksum",
	0,
	OPT_checksum,
	"Generate a \amethod\a checksum file for each archive member and add\
	the resulting file as the archive member \apath\a. See \bsum\b(1) for\
	supported checksum methods; \bmd5\b is a good candidate. The generated\
	file is suitable for input to \bsum --check --permissions\b.",
	"method::path",
},
{
	"chksum",
	0,
	OPT_chksum,
	"The header checksum string; empty if not supported.",
	0,
	0,
	OPT_READONLY,
},
{
	"chmod",
	'C',
	OPT_chmod,
	"Apply the \bchmod\b(1) \amode\a expression to file modes\
	written to archive headers.",
	"mode"
},
{
	"clobber",
	'k',
	OPT_clobber,
	"Overwrite output files.",
	0,
	0,
	OPT_INVERT,
},
{
	"comment",
	0,
	OPT_comment,
	"Comment text.",
	"text",
	0,
	OPT_HEADER,
},
{
	"complete",
	0,
	OPT_complete,
	"Complete archive must fit in one media part.",
},
{
	"crossdevice",
	'X',
	OPT_crossdevice,
	"Directory traversal may cross device boundaries.",
	0,
	0,
	OPT_INVERT,
},
{
	"ctime",
	0,
	OPT_ctime,
	"Preserve or set file change times.",
	"time",
	0,
	OPT_HEADER|OPT_OPTIONAL,
},
{
	"debug",
	'D',
	OPT_debug,
	"Set debug trace level. Higher levels produce more output.",
	"level",
	0,
	OPT_NUMBER,
},
{
	"delete",
	0,
	OPT_delete,
	"\bdelete\b=\apattern\a ignores all global and extended header keywords\
	matching the \bksh\b(1) \apattern\a.",
	"pattern",
},
{
	"delta.base.checksum",
	0,
	OPT_delta_base_checksum,
	"The delta base archive checksum.",
	"checksum",
	0,
	OPT_GLOBAL|OPT_READONLY|OPT_NUMBER|OPT_VENDOR,
},
{
	"delta.base.size",
	0,
	OPT_delta_base_size,
	"The delta base archive size.",
	"size",
	0,
	OPT_GLOBAL|OPT_READONLY|OPT_NUMBER|OPT_VENDOR,
},
{
	"delta.checksum",
	0,
	OPT_delta_checksum,
	"The delta archive checksum not including the current entry.",
	"checksum",
	0,
	OPT_GLOBAL|OPT_READONLY|OPT_NUMBER|OPT_VENDOR,
},
{
	"delta.compress",
	0,
	OPT_delta_compress,
	"The delta base archive is \b/dev/null\b.",
	0,
	0,
	OPT_GLOBAL|OPT_READONLY|OPT_VENDOR,
},
{
	"delta.index",
	0,
	OPT_delta_index,
	"The base file index (ordinal) of the current delta archive entry.",
	"index",
	0,
	OPT_GLOBAL|OPT_READONLY|OPT_NUMBER|OPT_VENDOR,
},
{
	"delta.method",
	0,
	OPT_delta_method,
	"The delta method. Supported methods are:",
	"method",
	"\fDELTA\f",
	OPT_GLOBAL|OPT_VENDOR,
},
{
	"delta.op",
	0,
	OPT_delta_op,
	"The current delta archive entry read mode operation:",
	"op",
	"[c:create?The file is not in the base archive and will be created.]\
	 [d:delete?The file will be deleted.]\
	 [p:pass?The file is not a delta and will be copied verbatim.]\
	 [u:update?The file is a delta and will be updated against the\
		corresponding base archive entry.]\
	 [v:verify?No data change but the file metatdata will be updated\
		to match the delta archive.]\
	",
	OPT_GLOBAL|OPT_READONLY|OPT_VENDOR,
},
{
	"delta.ordered",
	0,
	OPT_delta_ordered,
	"The base and delta archive members are ordered (sorted) by name.",
	0,
	0,
	OPT_GLOBAL|OPT_READONLY|OPT_VENDOR,
},
{
	"delta.update",
	0,
	OPT_delta_update,
	"Only update files in the delta -- do not update files in the base that are not in the delta.",
	0,
	0,
	OPT_GLOBAL|OPT_VENDOR,
},
{
	"delta.version",
	0,
	OPT_delta_version,
	"The delta method version.",
	"version",
	0,
	OPT_GLOBAL|OPT_READONLY|OPT_VENDOR,
},
{
	"descend",
	'd',
	OPT_descend,
	"Command line directories name themselves and descendents.",
	0,
	0,
	OPT_INVERT,
},
{
	"device",
	0,
	OPT_device,
	"The device major and minor number string, empty if not a device file.",
	0,
	0,
	OPT_READONLY,
},
{
	"devmajor",
	0,
	OPT_devmajor,
	"The major device number, 0 if not supported.",
	0,
	0,
	OPT_READONLY,
},
{
	"devminor",
	0,
	OPT_devminor,
	"The minor device number, 0 if not supported.",
	0,
	0,
	OPT_READONLY,
},
{
	"different",
	'U',
	OPT_different,
	"Only copy archive members that have different modify time, mode, or size\
	than the target files.  Target file names are checked after \b--edit\b\
	options are applied.",
},
{
	"dir",
	0,
	OPT_dir,
	"File directory name (base elided).",
	0,
	0,
	OPT_READONLY,
},
{
	"dots",
	0,
	OPT_dots,
	"Print a dot on the standard error for each block transferred.",
},
{
	"edit",
	's',
	OPT_edit,
	"Pathname substitution from \aold\a to \anew\a.\
	The first character is the expression delimiter.\
	There may be more than one edit expression;\
	each is applied in order from left to right.",
	",old,new,[glpsu]][i]]",
	"[+g?All \aold\a patterns.]\
	[+l?Convert \anew\a to lower case.]\
	[+p?Print the edit result on the standard error.]\
	[+s?Stop edits on path if this edit succeeds.]\
	[+u?Convert \anew\a to upper case.]\
	[+i?Append member index to pathname in \b.%04d format\b.]",
},
{
	"entry",
	0,
	OPT_entry,
	"File entry ordinal.",
	0,
	0,
	OPT_HEADER,
},
{
	"eom",
	0,
	OPT_eom,
	"End of media prompt or !\acommand\a. Processing terminates if\
	\b!\b\acommand\a returns non-zero exit status or if \b!\b is\
	specified.",
	"[!]][prompt]]",
},
{
	"exact",
	'n',
	OPT_exact,
	"Exit after each file arg matches exactly once.",
},
{
	"exthdr.name",
	0,
	OPT_extended_name,
	"Equivalent to \bheader\b:=\aformat\a.",
	"format",
},
{
	"file",
	'f',
	OPT_file,
	"The main archive file name.",
	"path",
},
{
	"filter",
	0,
	OPT_filter,
	"Input/output file filter command. \acommand\a is applied to each\
	file as it is read from or written to the archive. \acommand\a is\
	split into space separated arguments, and is executed with the\
	pathname of the file to be processed as the last argument.\
	The standard output of the resulting command is read by \bpax\b.\
	\b--nodescend\b is implied by \b--filter\b. If \acommand\a is \b-\b\
	and the archive is being written and there are no command line\
	\afile\a arguments, then each line on the standard input is\
	interpreted as a delimiter separated command:\
	\bX\b\aoptions\a\bX\b\acommand\a\bX\b\aphysical\a\bX\b\alogical\a,\
	where:",
	"command",
	"[+X?A delimiter character that does not appear outside quotes.]\
	[+options?\b,\b separated [\bno\b]]\aname\a[=\avalue\a]] options:]{\
		[+logical?Override the command line \b--logical\b and\
			\b--physical\b options for this file.]\
		[+physical?Override the command line \b--logical\b and\
			\b--physical\b options for this file.]\
	}\
	[+command?A shell command that reads the physical file and writes\
		the filtered contents to the standard output. If \acommand\a\
		is empty then the file contents are copied unchanged.]\
	[+physical?The actual file path, used for archive status.]\
	[+logical?The file path as it will appear in the archive. If\
		\alogical\a is empty then the \aphysical\a path is used. The\
		resulting path is still subject to any \b--edit\b options.]"
},
{
	"forceconvert",
	0,
	OPT_forceconvert,
	"Force \b--from\b conversion even if the data contains control\
	characters in the first 256 bytes.",
},
{
	"format",
	'x',
	OPT_format,
	"The archive format. Formats and compression methods are automatically\
	detected on read. A basic, compress and delta format may be combined,\
	separated by \b:\b. Each format may be followed by =\bdetails\b; details\
	are format specific. \bt\b\acompress\a is shorthand for \btar\b:\acompress\a.\
	The supported formats are:",
	"format:=" FMT_DEFAULT,
	"\fformats\f",
},
{
	"from",
	0,
	OPT_from,
	"File data input character set name.\
	Only files that have no control characters in the first 256 bytes\
	are converted. See \b--charset\b for supported character set names.",
	"charset",
	"\fcharsets\f",
},
{
	"gid",
	0,
	OPT_gid,
	"Group id. The default is the group id of the invoking process.",
	"group",
	0,
	OPT_HEADER|OPT_NUMBER,
},
{
	"globexthdr.name",
	0,
	OPT_global_name,
	"Equivalent to \bheader\b=\aformat\a.",
	"format",
},
{
	"gname",
	0,
	OPT_gname,
	"Group name. The default is the group name of the invoking process.",
	"group",
	0,
	OPT_HEADER,
},
{
	"hdrcharset",
	0,
	OPT_hdrcharset,
	"The name of the character set used to encode text fields in pax extended\
	header record text fields. See \b--charset\b for supported character\
	set names.",
	"charset",
	0,
	OPT_HEADER,
},
{
	"header",
	0,
	OPT_header,
	"\bheader\b=\aformat\a sets the global header path name format to the\
	\blistopt\b \aformat\a. The default is \b" HEADER_EXTENDED_STD "\b\
	when strict conformance is in effect (see \bgetconf\b(1) CONFORMANCE)\
	and \b" HEADER_EXTENDED "\b otherwise. \bheader\b:=\aformat\a\
	sets the extended header path name format to the \blistopt\b\
	\aformat\a. The default is \b" HEADER_GLOBAL_STD "\b when strict\
	conformance is in effect and \b" HEADER_GLOBAL "\b otherwise.\
	The strict conformance defaults are prone to global header filename\
	collisions and are ill-defined when extended header names exceed the\
	underlying header format limits.",
	"format",
},
{
	"ignore",
	0,
	OPT_ignore,
	"\bignore\b=\apattern\a ignores all global and extended header keywords\
	matching the \bksh\b(1) \apattern\a; \bignore\b:=\apattern\a ignores\
	all extended header keywords matching \apattern\a.",
	"pattern",
},
{
	"ino",
	0,
	OPT_ino,
	"The file serial number, 0 if not supported.",
	0,
	0,
	OPT_READONLY,
},
{
	"install",
	0,
	OPT_install,
	"Generate an installation \bsh\b(1) script file that contains\
	\bchmod\b(1), \bchgrp\b(1) and \bchown\b(1) commands to restore\
	file modes and ownership not supported by standard \bcpio\b(1)\
	and \btar\b(1). Only files with user or group specific\
	read/execute/setuid permissions are included in the script.\
	The script is added to the archive with member name \apath\a.",
	"path",
},
{
	"intermediate",
	'I',
	OPT_intermediate,
	"Copy each file to an intermediate name and rename to the\
	real name if the intermediate copy succeeds; otherwise retain\
	the original file and issue a diagnostic.",
},
{
	"invalid",
	0,
	OPT_invalid,
	"Invalid path action:",
	"action",
	"[b:binary?hdrcharset=BINARY extended header record for unencodable data.]\
	[i:bypass|ignore?Silently ignore.]\
	[p:rename|prompt?Prompt for new name.]\
	[t:write|translate?Automatically translate and/or truncate\
		to local specifications.]\
	[u:utf-8|UTF-8?Convert to UTF-8.]",
},
{
	"invert",
	'c',
	OPT_invert,
	"Invert pattern match sense. The !(...) construct is more general.",
},
{
	"keepgoing",
	'K',
	OPT_keepgoing,
	"Attempt to skip over damaged input archive data.",
},
{
	"label",
	0,
	OPT_label,
	"Append \astring\a to the volume label; \blabel\b:=\astring\a\
	prepends \astring\a.",
	"string",
},
{
	"link",
	'l',
	OPT_link,
	"Hard link files on output if possible.",
},
{
	"linkdata",
	0,
	OPT_linkdata,
	"Output data with each hard link. The default outputs the\
	data only with the first link in the archive.",
},
{
	"linkop",
	0,
	OPT_linkop,
	"The link operation string, \b==\b for hard links, \b->\b for\
	symbolic links, otherwise empty.",
	0,
	0,
	OPT_READONLY,
},
{
	"linkpath",
	0,
	OPT_linkpath,
	"Symbolic link pathname.",
	"path",
	0,
	OPT_HEADER,
},
{
	"listformat|listopt",
	0,
	OPT_listformat,
	"Append to the member listing format string. \aformat\a follows\
	\bprintf\b(3) conventions, except that \bsfio\b(3) inline ids\
	are used instead of arguments:\
	%[-+]][\awidth\a[.\aprecis\a[.\abase\a]]]]]](\aid\a[:\asubformat\a]])\achar\a.\
	If \achar\a is \bs\b then the string form of the item is listed,\
	otherwise the corresponding numeric form is listed. \asubformat\a\
	overrides the default formatting for \aid\a. All of the file related\
	options are supported as \aid\as, along with the following:",
	"format",
	"\flistformat\f",
},
{
	"listmacro",
	0,
	OPT_listmacro,
	"Define a \b--listformat\b macro.",
	"name[=value]]",
},
{
	"local",
	0,
	OPT_local,
	"Reject files and links that traverse outside the current directory.",
},
{
	"logical|follow",
	'L',
	OPT_logical,
	"Follow symbolic links. The default is determined by\
	\bgetconf PATH_RESOLVE\b.",
},
{
	"magic",
	0,
	OPT_magic,
	"The header magic string; empty if not supported.",
	0,
	0,
	OPT_READONLY,
},
{
	"mark",
	0,
	OPT_mark,
	"The file type mark character string:",
	0,
	"[+=?hard link]\
	[+@?symbolic link]\
	[+/?directory]\
	[+|?fifo]\
	[+=?socket]\
	[+$?block or character special]\
	[+*?executable]",
	OPT_READONLY,
},
{
	"maxout",
	0,
	OPT_maxout,
	"Output media size limit.",
	"size",
	0,
	OPT_NUMBER,
},
{
	"metaphysical",
	'H',
	OPT_metaphysical,
	"Follow command argument symbolic links, otherwise don't follow.\
	The default is determined by \bgetconf PATH_RESOLVE\b.",
},
{
	"meter",
	'm',
	OPT_meter,
	"Display a one line text meter showing archive read progress.\
	The input must be seekable. \bcompress\b and \bbzip\b uncompressed\
	input size is estimated.",
},
{
	"mkdir",
	0,
	OPT_mkdir,
	"Create intermediate directories on output. On by default.",
},
{
	"mode",
	0,
	OPT_mode,
	"The file type and acces mode.",
	0,
	0,
	OPT_READONLY,
},
{
	"mtime",
	0,
	OPT_mtime,
	"Preserve or set file modify times.",
	"time",
	0,
	OPT_HEADER|OPT_OPTIONAL,
},
{
	"name",
	0,
	OPT_name,
	"File base name (directory elided).",
	0,
	0,
	OPT_HEADER,
},
{
	"newer",
	'N',
	OPT_newer,
	"Only copy archive members that are newer than the target files.\
	Target file names are checked after \b--edit\b options are applied.",
},
{
	"nlink",
	0,
	OPT_nlink,
	"The hard link count.",
	0,
	0,
	OPT_READONLY,
},
{
	"options",
	'o',
	OPT_options,
	"Set options without leading -- from \astring\a.",
	"string",
},
{
	"ordered",
	'O',
	OPT_ordered,
	"Input files and base are ordered by name.",
},
{
	"owner",
	0,
	OPT_owner,
	"Change output file owner to current user or to \auid\a\
	if specified.",
	"uid",
},
{
	"passphrase",
	'E',
	OPT_passphrase,
	"Passphrase for formats that support encryption. The default value is"
	" prompted on and read from \b/dev/tty\b.",
	"passphrase",
},
{
	"path",
	0,
	OPT_path,
	"File path name.",
	"path",
	0,
	OPT_HEADER,
},
{
	"physical",
	'P',
	OPT_physical,
	"Don't follow symbolic links.\
	The default is determined by \bgetconf PATH_RESOLVE\b.",
},
{
	"pid",
	0,
	OPT_pid,
	"\bpax\b process id.",
},
{
	"preserve",
	'p',
	OPT_preserve,
	"Preserve selected file attributes:",
	"aemops",
	"[+a?Don't preserve access time.]\
	[+e?Preserve everything permissible.]\
	[+m?Don't preserve modify time.]\
	[+o?Preserve user, group, setuid and setgid.]\
	[+p?Preserve mode.]\
	[+s?Preserve setuid and setgid.]",
	OPT_OPTIONAL,
},
{
	"read",
	'r',
	OPT_read,
	"Read files from the archive.",
},
{
	"record-charset",
	0,
	OPT_record_charset,
	"Enable character set translation. On by default.",
},
{
	"record-delimiter",
	0,
	OPT_record_delimiter,
	"\bvdb\b format record delimiter character.\
	No delimiter if omitted. The default is ; .",
	"char",
},
{
	"record-format",
	0,
	OPT_record_format,
	"Labeled tape record format:",
	"DFSUV",
	"[+D?decimal variable]\
	[+F?fixed length]\
	[+S?spanned]\
	[+U?input block size]\
	[+B?binary variable]",
},
{
	"record-header",
	0,
	OPT_record_header,
	"Member header, NULL if omitted. The default value is format specific.",
},
{
	"record-line",
	0,
	OPT_record_line,
	"Records are lines. The default is format specific.",
},
{
	"record-match",
	0,
	OPT_record_match,
	"Select record formats that match \apattern\a.",
	"pattern",
},
{
	"record-pad",
	0,
	OPT_record_pad,
	"Pad records. The default is format specific.",
},
{
	"record-size",
	0,
	OPT_record_size,
	"Fixed length record size. The default is format specific.",
	"size",
	0,
	OPT_NUMBER,
},
{
	"record-trailer",
	0,
	OPT_record_trailer,
	"Member trailer, NULL if omitted. The default is format specific.",
	"string",
},
{
	"release",
	0,
	OPT_release,
	"The \bpax\b implementation release stamp.",
	"string",
	0,
	OPT_GLOBAL|OPT_READONLY|OPT_IGNORE|OPT_VENDOR,
},
{
	"reset-atime",
	't',
	OPT_reset_atime,
	"Reset the file access times of copied files.",
},
{
	"sequence",
	0,
	OPT_sequence,
	"The archive member sequence number. Numbers for volumes >1 are of the\
	form \avolume\a.\asequence\a.",
	0,
	0,
	OPT_READONLY,
},
{
	"size",
	0,
	OPT_size,
	"File size.",
	"size",
	0,
	OPT_HEADER|OPT_OPTIONAL,
	4
},
{
	"strict",
	'S',
	OPT_strict,
	"Disable non-standard extensions. The default is determined\
	by the \bgetconf\b(1) CONFORMANCE setting.",
},
{
	"summary",
	0,
	OPT_summary,
	"List summary information for each archive. On by default.",
},
{
	"symlink",
	0,
	OPT_symlink,
	"symlink files if possible.",
},
{
	"sync|fsync",
	'F',
	OPT_sync,
	"\bfsync\b(2) each file after it is copied.",
},
{
	"tape",
	0,
	OPT_tape,
	"Canonical tape unit name and operations.",
	"[#]][lmhcu]][n]][bv]][s[#]]]][k[#]]]]",
	"[+#?unit number [0-9]]]\
	[+l?low density]\
	[+m?medium density]\
	[+h?high density]\
	[+c?compressed]\
	[+u?uncompressed]\
	[+n?don't rewind on close]\
	[+b?bsd behavior]\
	[+v?system V behavior]\
	[+s[#]]?skip all [#]] volumes]\
	[+k[#]]?keep all [#]] volumes after skip]",
},
{
	"test",
	'T',
	OPT_test,
	"Test mask for debugging. Multiple values are or'ed together.",
	"mask",
	0,
	OPT_NUMBER,
},
{
	"testdate",
	0,
	OPT_testdate,
	"\b--listformat\b time values newer than \adate\a will be printed\
	as \adate\a. Used for regression testing.",
	"date",
},
{
	"times",
	0,
	OPT_times,
	"Preserve \batime\b, \bctime\b, and \bmtime\b.",
},
{
	"tmp",
	0,
	OPT_tmp,
	"The value of the \bTMPDIR\b environment variable if defined,\
	otherwise \b/tmp\b.",
},
{
	"to",
	0,
	OPT_to,
	"Output character set. See \b--charset\b for supported\
	character set names.",
	"name",
},
{
	"typeflag",
	0,
	OPT_typeflag,
	"The header type flag string; empty if not supported.",
	0,
	0,
	OPT_READONLY,
},
{
	"uid",
	0,
	OPT_uid,
	"User id. The default is the user id of the invoking process.",
	"user",
	0,
	OPT_HEADER|OPT_NUMBER,
},
{
	"uname",
	0,
	OPT_uname,
	"User name. The default is the user name of the invoking process.",
	"user",
	0,
	OPT_HEADER,
},
{
	"unblocked",
	0,
	OPT_unblocked,
	"Force unblocked input/output. The default is format specific.\
	Both input and output are unblocked if the value is omitted.",
	"i|o",
},
{
	"uncompressed|delta.size",
	0,
	OPT_uncompressed,
	"The uncompressed size of the current archive entry, 0 if the\
	entry is not compressed.",
	"size",
	0,
	OPT_GLOBAL|OPT_READONLY|OPT_NUMBER|OPT_VENDOR,
},
{
	"update",
	'u',
	OPT_update,
	"Only copy archive members that are newer than the target files.\
	Target file names are checked before \b--edit\b options are applied.",
},
{
	"verbose",
	'v',
	OPT_verbose,
	"Produce long listings or list each file name as it is processed.",
},
{
	"verify",
	'i',
	OPT_verify,
	"Prompt for verification and/or rename.",
},
{
	"version",
	0,
	OPT_version,
	"The header version string; empty if not supported.",
	0,
	0,
	OPT_READONLY,
},
{
	"warn",
	'W',
	OPT_warn,
	"Enable archive format specific warnings.",
},
{
	"write",
	'w',
	OPT_write,
	"Write files to the archive.",
},
{
	"yes",
	'y',
	OPT_yes,
	"Prompt for yes/no file verification.",
},
{ 0 }
};
