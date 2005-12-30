# Copyright 2005 David Faure <faure@kde.org>
# BSD license (see COPYING)

"""
This tool is used to find and load the libxml2 and libxslt
neceessary compilation and link flags
"""

def exists(env):
	return true

#
# Check for a function (e.g. usleep or strlcat) for which we have a replacement available
# in kdecore/fakes.c (libkdefakes).
# (This is a replacement for KDE_CHECK_FUNC_EXT)
def CheckFuncWithKdefakeImpl(context, dest, function_name, header, sample_use, prototype):
	code="""
%(header)s
int main() {
	%(sample_use)s;
	return 0;
}
	""" % { 'header' : header, 'sample_use' : sample_use }
	context.Message("Checking for function %s()... " % (function_name))
	ret = context.TryLink(code, '.cpp')
	if ret:
		import string
		dest.write("#define HAVE_%s 1\n" % (string.upper(function_name)))
		context.Result(ret)
	else:
                context.Result('ok - in libkdefakes')
                dest.write("""
/* No %(function_name)s function found on the system, kdefakes will provide it. */
#ifdef __cplusplus
extern "C" {
#endif
%(prototype)s;
#ifdef __cplusplus
}
#endif
""" % { 'prototype' : prototype, 'function_name' : function_name } )
	return ret


def generate(env):
	if env['HELP']:
		print "no help for kdefakes"
		return

	import sys

	# tell it we do have a global config.h file
	env['_CONFIG_H_'].append('kdefakes')

	from SCons.Options import Options
	cachefile=env['CACHEDIR']+'kdefakes.cache.py'
	opts = Options(cachefile)
	opts.AddOptions(
		('KDEFAKES_ISCONFIGURED', ''),
	)
	opts.Update(env)

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('KDEFAKES_ISCONFIGURED')):
		env['_CONFIGURE_']=1
		import sys
		import os

		dest=open(env.join(env['_BUILDDIR_'], 'config-kdefakes.h'), 'w')
		dest.write('/* kdefakes configuration created by bksys */\n\n#include <kdelibs_export.h>\n')

		conf = env.Configure( custom_tests = { 'CheckFuncWithKdefakeImpl' : CheckFuncWithKdefakeImpl } )
		## TODO let the caller specify which checks they want? Hopefully not turning this into one-file-per-check though...
		conf.CheckFuncWithKdefakeImpl(dest, 'setenv', '#include <stdlib.h>',
			'setenv("VAR", "VALUE", 1);',
			'KDECORE_EXPORT int setenv (const char *, const char *, int)')
		conf.CheckFuncWithKdefakeImpl(dest, 'unsetenv', '#include <stdlib.h>',
			'unsetenv("VAR");',
			'KDECORE_EXPORT void unsetenv (const char *)')
		conf.CheckFuncWithKdefakeImpl(dest, 'getdomainname', '#include <stdlib.h>\n#include <unistd.h>\n#include <netdb.h>',
			'char buffer[200]; getdomainname(buffer, 200);',
			'#include <sys/types.h>\n'
			'KDECORE_EXPORT int getdomainname (char *, size_t)')
		# TODO: does not work yet on mingw
		if not env['WINDOWS']:
			conf.CheckFuncWithKdefakeImpl(dest, 'gethostname', '#include <stdlib.h>\n#include <unistd.h>',
				'char buffer[200]; gethostname(buffer, 200);',
				'int gethostname (char *, unsigned int)')
			conf.CheckFuncWithKdefakeImpl(dest, 'usleep', '#include <unistd.h>',
				'sleep (200);',
				'int usleep (unsigned int)')
			conf.CheckFuncWithKdefakeImpl(dest, 'random', '#include <stdlib.h>',
				'random();',
				'long int random(void)')
			conf.CheckFuncWithKdefakeImpl(dest, 'srandom', '#include <stdlib.h>',
				'srandom(27);',
				'void srandom(unsigned int)')
		conf.CheckFuncWithKdefakeImpl(dest, 'initgroups', '#include <sys/types.h>\n#include <unistd.h>\n#include <grp.h>',
			'char buffer[200]; initgroups(buffer, 27);',
			'int initgroups(const char *, gid_t)')
		conf.CheckFuncWithKdefakeImpl(dest, 'mkstemps', '#include <stdlib.h>\n#include <unistd.h>',
			'mkstemps("/tmp/aaaXXXXXX", 6);',
			'KDECORE_EXPORT int mkstemps(char *, int)')
		conf.CheckFuncWithKdefakeImpl(dest, 'mkstemp', '#include <stdlib.h>\n#include <unistd.h>',
			'mkstemp("/tmp/aaaXXXXXX");',
			'KDECORE_EXPORT int mkstemp(char *)')
		conf.CheckFuncWithKdefakeImpl(dest, 'mkdtemp', '#include <stdlib.h>\n#include <unistd.h>',
			'mkdtemp("/tmp/aaaXXXXXX");',
			'KDECORE_EXPORT char* mkdtemp(char *)')
		conf.CheckFuncWithKdefakeImpl(dest, 'strlcpy', '#include <string.h>',
			'char buf[20]; strlcpy(buf, "KDE function test", sizeof(buf));',
			'KDECORE_EXPORT unsigned long strlcpy(char*, const char*, unsigned long)')
		conf.CheckFuncWithKdefakeImpl(dest, 'strlcat', '#include <string.h>',
			'char buf[20]; buf[0]=0; strlcat(buf, "KDE function test", sizeof(buf));',
			'KDECORE_EXPORT unsigned long strlcat(char*, const char*, unsigned long)')
		# TODO finish (AC_CHECK_RES_QUERY and AC_CHECK_DN_SKIPNAME)
		# TODO AC_CHECK_RES_INIT is a bit more complicated

		env = conf.Finish()

		dest.close()

		env['KDEFAKES_ISCONFIGURED']=1
		opts.Save(cachefile, env)

