# Copyright Thomas Nagy 2005
# BSD license (see COPYING)

"""
This tool is used to find and load the libxml2 and libxslt
neceessary compilation and link flags
"""

def exists(env):
	return true

"""
Checks for all needed (non platform specific)headers and
defines HAVE_<HEADERNAME> if found
"""
def checkForHeaders(env, dest):
	import re
	def define_line(define, bool):
		if bool:
			return "#define " + define + " 1\n\n"
		else:
			return "/* #undef " + define + " */\n\n"

	headers = [
		'alloca.h',
		'arpa/nameser8_compat.h',
		'Carbon/Carbon.h',
		'CoreAudio/CoreAudio.h',
		'crt_externs.h',
		'ctype.h',
		'cups/cups.h',
		'dirent.h',
		'dl.h',
		'dld.h',
		'dlfcn.h',
		'errno.h',
		'float.h',
		'fstab.h',
		'ieeefp.h',
		'inttypes.h',
		'libutil.h',
		'limits.h',
		'locale.h',
		'malloc.h',
		'memory.h',
		'mntent.h',
		'ndir.h',
		'netinet/in.h',
		'nl_types.h',
		'pam/pam_appl.h',
		'paths.h',
		'pty.h',
		'security/pam_appl.h',
		'stdint.h',
		'stdio.h',
		'stdlib.h',
		'string.h',
		'strings.h',
		'sysent.h',
		'sys/bitypes.h',      ## ?
		'sys/bittypes.h',     ## ?
		'sys/dir.h',
		'sys/filio.h',
		'sys/mntent.h',
		'sys/mnttab.h',
		'sys/mount.h',
		'sys/ndir.h',
		'sys/param.h',
		'sys/select.h',
		'sys/soundcard.h',
		'sys/stat.h',
		'sys/stropts.h',
		'sys/time.h',
		'sys/timeb.h',
		'sys/types.h',
		'sys/ucred.h',
		'termio.h',
		'termios.h',
		'unistd.h',
		'util.h',
		'values.h',
		'X11/extensions/shape.h',
		'X11/extensions/Xshm.h',
		'X11/ICE/icelib.h',
	]
	dep_headers = [
		['sys/types.h','sys/socket.h','net/if.h'],
		['sys/types.h','sys/mman.h'],
	]

	# why do we need this?
	if not env['WINDOWS']:
		env['CPPPATH'] += [ '/usr/include','/usr/local/include' ]

	conf = env.Configure()

	content = ""
	define_regex = re.compile("(\\.|\\/|\\\\)")
	for header in headers:
		header_define = "HAVE_" + define_regex.sub('_', header).upper()
		content += "/* Define to 1 if you have the <" + header + "> header file. */\n"
		content += define_line(header_define, conf.CheckHeader(header))

	for dep_headers in dep_headers:
		header = dep_headers[-1]
		header_define = "HAVE_" + define_regex.sub('_', header).upper()
		content += "/* Define to 1 if you have the <" + header + "> header file. */\n"
		content += define_line(header_define, conf.CheckHeader(header))

	dest.write(content)
	env = conf.Finish()


## TODO documentation
##############  -- IMPORTANT --
## warning: the naming matters, see genobj -> uselib in generic.py
## warning: parameters are lists
## obj.uselib='Z PNG KDE4' will add all the flags
##   and paths from zlib, libpng and kde4 vars defined below


## TODO move to platform lowlevel.py
## DF: only the platform-dependent parts, right?
def generate(env):
	if env['HELP']:
		print "no help for lowlevel"
		return

	import sys

	#######################################
	## other stuff

	if env['WINDOWS']:
		env['CONVENIENCE']         = []
	else:
		if sys.platform == 'darwin':
			env['CONVENIENCE']         = ['-fPIC','-DPIC','-fno-common'] # TODO flags for convenience libraries
		else:
			env['CONVENIENCE']         = ['-fPIC','-DPIC'] # TODO flags for convenience libraries

	"""
		Shouldn't this dependency go into libpng.py ?
		e.g. png defines that it needs zlib and after all checks we have to look if zlib is available
	"""
	########## zlib
	if env['WINDOWS'] and env['CC'] == 'cl':
		env['LIB_Z']         = ['zlib']
	else:
		env['LIB_Z']         = ['z']

	########## png
	env['LIB_PNG']             = ['png', 'm'] + env['LIB_Z']

	if not env['WINDOWS'] and not env['MAC']:
		########## sm
		env['LIB_SM']              = ['SM', 'ICE']
	
		env['LIB_DL']              = ['dl']

		########## X11
		env['LIB_X11']             = ['X11']
		env['LIBPATH_X11']         = ['/usr/X11R6/lib/']
		env['LIB_XRENDER']         = ['Xrender']

	# tell it we do have a global config.h file
	env['_CONFIG_H_'].append('lowlevel')

	from SCons.Options import Options
	cachefile=env['CACHEDIR']+'lowlevel.cache.py'
	opts = Options(cachefile)
	opts.AddOptions(
		('LOWLEVEL_ISCONFIGURED', ''),
	)
	opts.Update(env)

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('LOWLEVEL_ISCONFIGURED')):
		env['_CONFIGURE_']=1
		import sys
		import os

		## Didn't generic.py do that already?
		if not os.path.exists( env['_BUILDDIR_'] ): os.mkdir( env['_BUILDDIR_'] )

		dest=open(env.join(env['_BUILDDIR_'], 'config-lowlevel.h'), 'w')
		dest.write('/* lowlevel configuration created by bksys */\n\n')

		if not env['WINDOWS']:
			dest.write("\n/* What OS used for compilation */\n#define KDE_COMPILING_OS \"%s %s %s\"\n\n" 
				% (os.uname()[0], os.uname()[2], os.uname()[4]))

		checkForHeaders(env, dest)

		sys.path.insert(0,env.getBksysPlatformPath(sys))
		from detect_lowlevel import detect

		detect(env, dest)
		dest.close()

		env['LOWLEVEL_ISCONFIGURED']=1
		opts.Save(cachefile, env)

