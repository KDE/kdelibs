##
# @file 
# platform-dependent low level configuration

def detect(lenv, dest):
	import os

	def CheckAndWrite( conf, header ):
		have_define = 'HAVE_' + header.upper()
		have_define = str.replace( have_define, '/', '_' )
		have_define = str.replace( have_define, '\\', '_' )
		have_define = str.replace( have_define, '.', '_' )

		txt = '/* Define to 1 if you have the <' + header + '> header file. */\n'
		if conf.CheckHeader( header ):
			return txt + '#define '+ have_define + ' 1\n\n'
		else:
			return txt + '#undef ' + have_define + '\n\n'

	conf = lenv.Configure()
	content = ''
	content += CheckAndWrite( conf, 'ctype.h' )
	content += CheckAndWrite( conf, 'dirent.h' )
	content += CheckAndWrite( conf, 'dl.h' )
	content += CheckAndWrite( conf, 'dlfcn.h' )
	content += CheckAndWrite( conf, 'float.h' )
	content += CheckAndWrite( conf, 'fstab.h' )
	content += CheckAndWrite( conf, 'ieeefp.h' )
	content += CheckAndWrite( conf, 'inttypes.h' )
	content += CheckAndWrite( conf, 'libutil.h' )
	content += CheckAndWrite( conf, 'malloc.h' )
	content += CheckAndWrite( conf, 'memory.h' )
	content += CheckAndWrite( conf, 'mntent.h' )
	content += CheckAndWrite( conf, 'ndir.h' )
	content += CheckAndWrite( conf, 'netinet/in.h' )
	content += CheckAndWrite( conf, 'net/if.h' )
	content += CheckAndWrite( conf, 'nl_types.h' )
	content += CheckAndWrite( conf, 'paths.h' )
	content += CheckAndWrite( conf, 'pty.h' )
	content += CheckAndWrite( conf, 'string.h' )
	content += CheckAndWrite( conf, 'stdint.h' )
	content += CheckAndWrite( conf, 'stdio.h' )
	content += CheckAndWrite( conf, 'stdlib.h' )
	content += CheckAndWrite( conf, 'string.h' )
	content += CheckAndWrite( conf, 'strings.h' )
	content += CheckAndWrite( conf, 'sysent.h' )
	content += CheckAndWrite( conf, 'sys/bittypes.h' )
	content += CheckAndWrite( conf, 'sys/dir.h' )
	content += CheckAndWrite( conf, 'sys/filio.h' )
	content += CheckAndWrite( conf, 'sys/mman.h' )
	content += CheckAndWrite( conf, 'sys/mntent.h' )
	content += CheckAndWrite( conf, 'sys/mnttab.h' )
	content += CheckAndWrite( conf, 'sys/mount.h' )
	content += CheckAndWrite( conf, 'sys/ndir.h' )
	content += CheckAndWrite( conf, 'sys/param.h' )
	content += CheckAndWrite( conf, 'sys/select.h' )
	content += CheckAndWrite( conf, 'sys/soundcard.h' )
	content += CheckAndWrite( conf, 'sys/stat.h' )
	content += CheckAndWrite( conf, 'sys/stropts.h' )
	content += CheckAndWrite( conf, 'sys/time.h' )
	content += CheckAndWrite( conf, 'sys/timeb.h' )
	content += CheckAndWrite( conf, 'sys/types.h' )
	content += CheckAndWrite( conf, 'sys/ucred.h' )
	content += CheckAndWrite( conf, 'termios.h' )
	content += CheckAndWrite( conf, 'termio.h' )
	content += CheckAndWrite( conf, 'unistd.h' )
	content += CheckAndWrite( conf, 'util.h' )
	content += CheckAndWrite( conf, 'values.h' )

	lenv = conf.Finish()
	
	content+="""
#define LTDL_SHLIB_EXT ".dll"
#define LTDL_OBJDIR ""

/* Distribution Text to append to OS */
#define KDE_DISTRIBUTION_TEXT "KDE Libraries for MS Windows"
"""

	import sys
	wver=sys.getwindowsversion()
	content += """
/* What OS used for compilation */
#define KDE_COMPILING_OS \"MS Windows %d.%d %s\"
""" % (wver[0], wver[1], wver[4])

	dest.write(content)
