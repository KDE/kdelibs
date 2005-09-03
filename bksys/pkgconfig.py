# Copyright Thomas Nagy 2005
# BSD license (see COPYING)
def exists(env):
	return true

def generate(env):
	import SCons.Util, os

	# This funtion detects pkg-config
	def Check_pkg_config(context, version):

		from SCons.Options import Options
		
		optionFile = env['CACHEDIR'] + 'pkgconfig.cache.py'
		opts = Options(optionFile)
		opts.AddOptions(('PKGCONFIG_ISCONFIGURED', 'If pkgconfig is already tested for'))
		opts.Update(env)

		context.Message('Checking for pkg-config ... ')
			
		if not env.has_key('PKGCONFIG_ISCONFIGURED'):
			pkg_config_command = 'pkg-config'
			if os.environ.has_key("PKG_CONFIG_PATH"):
				pkg_config_command = "PKG_CONFIG_PATH="+os.environ["PKG_CONFIG_PATH"]+" pkg-config "
			ret = context.TryAction(pkg_config_command+' --atleast-pkgconfig-version=%s' % version)[0]
			env['PKGCONFIG_ISCONFIGURED'] = ret
			opts.Save(optionFile, env)
		else:
			ret = env['PKGCONFIG_ISCONFIGURED']
			
		context.Result(ret)
		
		return ret

	# This function detects a package using pkg-config
	def Check_package(context, variablePrefix, module, version):
		context.Message('Checking for %s >= %s ... ' % (module, version))
		pkg_config_command = 'pkg-config'
		if os.environ.has_key("PKG_CONFIG_PATH"):
			pkg_config_command = "PKG_CONFIG_PATH="+os.environ["PKG_CONFIG_PATH"]+" pkg-config "
		ret = context.TryAction(pkg_config_command+' %s --atleast-version=%s' % (module, version))[0]
		if ret:
			env.ParseConfig(pkg_config_command+' %s --cflags --libs' % module);
			env[variablePrefix + '_CFLAGS'] = SCons.Util.CLVar( 
				os.popen(pkg_config_command+" %s --cflags 2>/dev/null" % module).read().strip() );
			env[variablePrefix + '_LDFLAGS'] = SCons.Util.CLVar( 
					os.popen(pkg_config_command+" %s --libs 2>/dev/null" % module).read().strip() );
		context.Result(ret)
		return ret

	def findPkg(variablePrefix, module, version, env=env):
		from SCons.Options import Options

		optionFile = env['CACHEDIR'] + variablePrefix + '.cache.py'
		opts = Options(optionFile)
		opts.AddOptions(
				(variablePrefix +'_ISCONFIGURED', 'whether it is necessary to run configure or not'),
				(variablePrefix + '_CFLAGS', 'additional compilation flags'),
				(variablePrefix + '_LDFLAGS', 'link flags')
				)
		opts.Update(env)

		if not env.has_key(variablePrefix + '_ISCONFIGURED'):
			conf = env.Configure(custom_tests =
								 { 'Check_pkg_config' : Check_pkg_config,
								   'Check_package' : Check_package }
								 )

			if env.has_key(variablePrefix + '_CFLAGS'):
				env.__delitem__(variablePrefix + '_CFLAGS')
			if env.has_key(variablePrefix + '_LDFLAGS'):
				env.__delitem__(variablePrefix + '_LDFLAGS')

			if not conf.Check_pkg_config('0.15'):
				print 'pkg-config >= 0.15 not found.'
				env.Exit(1)

			haveModule = conf.Check_package(variablePrefix, module, version)
			env = conf.Finish()

			if haveModule:
				env[variablePrefix + '_ISCONFIGURED'] = 1

			opts.Save(optionFile, env)
		else:
			# if in cache, it worked
			haveModule = 1
			
		return haveModule

	env.pkgConfig_findPackage = findPkg;
