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

def generate(env):

	#######################################
	## other stuff
	env['CONVENIENCE']         = ['-fPIC','-DPIC'] # TODO flags for convenience libraries

	########## zlib
	env['LIB_Z']               = ['z']

	########## png
	env['LIB_PNG']             = ['png', 'z', 'm']

	########## sm
	env['LIB_SM']              = ['SM', 'ICE']

	env['LINKFLAGS_DL']        = ['-ldl']

	########## X11
	env['LIB_X11']             = ['X11']
	env['LIBPATH_X11']         = ['/usr/X11R6/lib/']
	env['LIB_XRENDER']         = ['Xrender']

	# TODO: the config.h stuff

