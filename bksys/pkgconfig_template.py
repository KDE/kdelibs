# Copyright Thomas Nagy 2005
# BSD license (see COPYING)

BOLD   ="\033[1m"
RED    ="\033[91m"
GREEN  ="\033[92m"
YELLOW ="\033[93m"
CYAN   ="\033[96m"
NORMAL ="\033[0m"

"""
Template for a bksys module that uses (optionally) pkg-config
Zones to adapt are marked with a TODO
Adapt as you see it fit
"""

"""
This tool is used to find and load XXX and XXX
neceessary compilation and link flags
"""

def exists(env):
	return true

def generate(env):
	import SCons.Util, os

	# This funtion detects pkg-config - DO NOT CHANGE
	def Check_pkg_config(context, version):
		context.Message('Checking for pkg-config ... ')
		pkg_config_command = 'pkg-config'
		if os.environ.has_key("PKG_CONFIG_PATH"):
			pkg_config_command = "PKG_CONFIG_PATH="+os.environ["PKG_CONFIG_PATH"]+" pkg-config "
		ret = context.TryAction(pkg_config_command+' --atleast-pkgconfig-version=%s' % version)[0]
		context.Result(ret)
		return ret

	# This function detects a package using pkg-config, DO NOT CHANGE
	def Check_package(context, module, version):
		context.Message('Checking for %s >= %s ... ' % (module, version))
		pkg_config_command = 'pkg-config'
		if os.environ.has_key("PKG_CONFIG_PATH"):
			pkg_config_command = "PKG_CONFIG_PATH="+os.environ["PKG_CONFIG_PATH"]+" pkg-config "
		ret = context.TryAction(pkg_config_command+' %s --atleast-version=%s' % (module, version))[0]
		if ret:
			env.ParseConfig(pkg_config_command+' %s --cflags --libs' % module);
			env.AppendUnique( SOUNDCCFLAGS = 
				SCons.Util.CLVar( 
					os.popen(pkg_config_command+" %s --cflags 2>/dev/null" % module).read().strip() ));
			env.AppendUnique( SOUNDLDFLAGS = 
				SCons.Util.CLVar( 
					os.popen(pkg_config_command+" %s --libs 2>/dev/null" % module).read().strip() ));
		context.Result(ret)
		return ret

	from SCons.Options import Options, PathOption
	# Our cache file
	# TODO give a name to your template and change this cache file name
	optionfile = env['CACHEDIR']+'mytemplate.cache.py'
	
	# Our options
	# TODO add your options
	opts = Options(optionfile)
	opts.AddOptions(
		( 'ISCONFIGURED', 'whether it is necessary to run configure or not' ),
		( 'TEMPLATE_CCFLAGS', 'additional compilation flags' ),
		( 'TEMPLATE_LDFLAGS', 'additional link flags' )
		)
	opts.Update(env)

	# detect the libxml and libxslt packages when needed
	# DO NOT CHANGE this condition
	if 'configure' in env['TARGS'] or not env.has_key('ISCONFIGURED'):
		conf = env.Configure(custom_tests = { 'Check_pkg_config' : Check_pkg_config, 'Check_package' : Check_package }) 

		# delete the cached variables
		# TODO add your options
		if env.has_key('TEMPLATE_CCFLAGS'):
			env.__delitem__('TEMPLATE_CCFLAGS')
		if env.has_key('TEMPLATE_LDFLAGS'):
			env.__delitem__('TEMPLATE_LDFLAGS')

		# detect if pkg-config is usable
		# TODO change the version number
		if not conf.Check_pkg_config('0.15'):
			print 'pkg-config >= 0.15 not found.' 
			env.Exit(1) 

		# check for the packages
		# TODO check *your* packages
		have_xml  = conf.Check_package('libxml-2.0', '2.6.0')
		have_xslt = conf.Check_package('libxslt', '1.1.0')

		env = conf.Finish()

		# if the config did not work issue a meaningful warning and exit
		# TODO add your validation below
		if not have_xml:
			print RED+'libxml-2.0 >= 2.6.0 was not found (mandatory).'+NORMAL
			env.Exit(1)

		if not have_xslt:
			print RED+'libxslt-2.0 >= 1.1.0 was not found (mandatory).'+NORMAL
			env.Exit(1)

		# mark the config as done - DO NOT CHANGE
		env['ISCONFIGURED'] = 1

		# store the config
		opts.Save(optionfile, env)

	# we just load the cached variables here - DO NOT CHANGE
	if env.has_key('TEMPLATE_CCFLAGS'):
		env.AppendUnique(CCFLAGS = env['TEMPLATE_CCFLAGS'] )
	if env.has_key('TEMPLATE_LDFLAGS'):
		env.AppendUnique(LINKFLAGS = env['TEMPLATE_LDFLAGS'] )

