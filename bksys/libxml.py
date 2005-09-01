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

	# the following two functions are used to detect the packages using pkg-config - scons feature
	def Check_pkg_config(context, version):
		context.Message('Checking for pkg-config ... ')
		pkg_config_command = 'pkg-config'
		if os.environ.has_key("PKG_CONFIG_PATH"):
			pkg_config_command = "PKG_CONFIG_PATH="+os.environ["PKG_CONFIG_PATH"]+" pkg-config "
		ret = context.TryAction(pkg_config_command+' --atleast-pkgconfig-version=%s' % version)[0]
		context.Result(ret)
		return ret

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

	# these are our options
	from SCons.Options import Options, PathOption
	cachefile = env['CACHEDIR']+'/xml.cache.py'
	opts = Options(cachefile)
	opts.AddOptions(
		( 'ISCONFIGURED', 'whether it is necessary to run configure or not' ),
		( 'XML_CCFLAGS', 'additional compilation flags' ),
		( 'XML_LDFLAGS', 'additional link flags' )
		)
	opts.Update(env)

	# detect the libxml and libxslt packages when needed
	if 'configure' in env['TARGS'] or not env.has_key('ISCONFIGURED'):
		conf = env.Configure(custom_tests = { 'Check_pkg_config' : Check_pkg_config, 'Check_package' : Check_package }) 

		# delete the cached variables
		if env.has_key('XML_CCFLAGS'):
			env.__delitem__('XML_CCFLAGS')
		if env.has_key('XML_LDFLAGS'):
			env.__delitem__('XML_LDFLAGS')

		if not conf.Check_pkg_config('0.15'):
			print 'pkg-config >= 0.15 not found.' 
			env.Exit(1) 

		have_xml  = conf.Check_package('libxml-2.0', '2.6.0')
		have_xslt = conf.Check_package('libxslt', '1.1.0')

		env = conf.Finish()

		# if the config worked, read the necessary variables and cache them
		if not have_xml:
			print RED+'libxml-2.0 >= 2.6.0 was not found (mandatory).'+NORMAL
			env.Exit(1)

		if not have_xslt:
			print RED+'libxslt-2.0 >= 1.1.0 was not found (mandatory).'+NORMAL
			env.Exit(1)

		# mark the config as done - libxml2 and libxslt are found
		env['ISCONFIGURED'] = 1

		# store the config
		opts.Save(cachefile, env)

	# we just load the cached variables here
	if env.has_key('XML_CCFLAGS'):
		env.AppendUnique(CCFLAGS = env['XML_CCFLAGS'] )
	if env.has_key('XML_LDFLAGS'):
		env.AppendUnique(LINKFLAGS = env['XML_LDFLAGS'] )

