# Copyright Thomas Nagy 2005
# BSD license (see COPYING)

"""
This tool is used to find and load the libxml2 and libxslt
neceessary compilation and link flags
"""

def exists(env):
	return true

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

		if sys.platform == 'darwin':
			sys.path.insert(0,'bksys'+os.sep+'osx')
			from detect_lowlevel import detect
		elif env['WINDOWS']:
			sys.path.insert(0,'bksys'+os.sep+'win32')
			from detect_lowlevel import detect
		else:
			sys.path.insert(0,'bksys'+os.sep+'unix')
			from detect_lowlevel import detect
		detect(env, dest)
		dest.close()

		env['LOWLEVEL_ISCONFIGURED']=1
		opts.Save(cachefile, env)

