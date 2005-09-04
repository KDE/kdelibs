# Copyright Thomas Nagy 2005
# BSD license (see COPYING)

BOLD   ="\033[1m"
RED    ="\033[91m"
GREEN  ="\033[92m"
YELLOW ="\033[93m"
CYAN   ="\033[96m"
NORMAL ="\033[0m"

"""
This tool is used to find and load the libxml2 and libxslt
neceessary compilation and link flags
"""

def exists(env):
	return true

def generate(env):
	import SCons.Util, os
	from SCons.Tool import Tool

	if env['HELP']: # nothing here
		return
	
	pkgs = Tool('pkgconfig', ['./bksys'])
	pkgs.generate(env)

	have_xml  = env.pkgConfig_findPackage('XML', 'libxml-2.0', '2.6.0')
	have_xslt = env.pkgConfig_findPackage('XSLT', 'libxslt', '1.1.0')

	# if the config worked, read the necessary variables and cache them
	if not have_xml:
		print RED+'libxml-2.0 >= 2.6.0 was not found (mandatory).'+NORMAL
		env.Exit(1)
		
	if not have_xslt:
		env.pprint('RED', 'libxslt-2.0 >= 1.1.0 was not found (mandatory).')
		env.Exit(1)

	# mark the config as done - libxml2 and libxslt are found
	env['ISCONFIGURED'] = 1

	# we just load the cached variables here
	if env.has_key('XML_CCFLAGS'):
		env.AppendUnique(CCFLAGS = env['XML_CCFLAGS'] )
	if env.has_key('XML_LDFLAGS'):
		env.AppendUnique(LINKFLAGS = env['XML_LDFLAGS'] )

