# Copyright Thomas Nagy 2005
# BSD license (see COPYING)
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
			env[module + '_CFLAGS'] = SCons.Util.CLVar( 
					os.popen(pkg_config_command+" %s --cflags 2>/dev/null" % module).read().strip() );
			env[module + '_LDFLAGS'] = SCons.Util.CLVar( 
					os.popen(pkg_config_command+" %s --libs 2>/dev/null" % module).read().strip() );
		context.Result(ret)
		return ret

	def findPkg(env, module, version):
		from SCons.Options import Options, PathOption

		optionFile = env['CACHEDIR'] + module + '.cache.py'
		opts = Options(optionFile)
		opts.AddOptions(
				(module +'_ISCONFIGURED', 'whether it is necessary to run configure or not'),
				(module + '_CFLAGS', 'additional compilation flags'),
				(module + '_LDFLAGS', 'link flags')
				)
		opts.Update(env)

		if not env.has_key(module + '_ISCONFIGURED'):
			conf = env.Configure(custom_tests = { 'Check_pkg_config' : Check_pkg_config,
							      'Check_package' : Check_package })

			if env.has_key(module + '_CFLAGS'):
				env.__delitem__(module + '_CFLAGS')
			if env.has_key(module + '_LDFLAGS'):
				env.__delitem__(module + '_LDFLAGS')

			if not conf.Check_pkg_config('0.15'):
				print 'pkg-config >= 0.15 not found.'
				env.Exit(1)

			haveModule = conf.Check_package(module, version)
			env = conf.Finish()

			if haveModule:
				env[module + '_ISCONFIGURED'] = 1

			opts.Save(optionFile, env)

		pkg = {}
		pkg['CFLAGS'] = env[module + '_CFLAGS']
		pkg['LDFLAGS'] = env[module + '_LDFLAGS']
		return pkg

	env.pkgConfig_findPackage = findPkg;
