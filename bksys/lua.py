# Copyright Scott Newton 2005
# BSD license (see COPYING)

true = 1
false = 0

def exists(env):
	return true

def generate(env):

	if env['WINDOWS']:
		return
	
	from SCons.Options import Options
	import os

	optionFile = env['CACHEDIR'] + 'lua.cache.py'
	opts = Options(optionFile)
	opts.AddOptions(
		('CACHED_LUA', 'Check whether the LUA library is available'),
		('CXXFLAGS_LUA',''),
		('LINKFLAGS_LUA',''),
		)
	opts.Update(env)
	checks_ok = true

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('CACHED_LUA')):
		conf = env.Configure( )
		env['CACHED_LUA'] = false;
		
		if not conf.CheckCHeader('lua.h'):
			checks_ok = false
			env.pprint('RED', 'lua.h (optional) was not found')

		if not conf.CheckCHeader('lualib.h'):
			checks_ok = false
			env.pprint('RED', 'lualib.h (optional) was not found')

		if checks_ok:
			if not conf.CheckLib(['lua', 'lualib'], 'lua_version'):
				checks_ok = false
				env.pprint('RED', 'Lua 5.0 libraries (optional) were not found')

		if checks_ok:
			luasrc = """
#include <lua.h>
#include <stdio.h>

int main(int argc, char * argv[])
{
	printf("%s", LUA_VERSION);
	return 0;
}
"""
			if conf.TryCompile(luasrc, '.c'):
				(status, version) = conf.TryRun(luasrc, '.c')
				if status:
					list = version.split()
					(major,minor,point) = list[1].split('.')
					if major >= 5:
						env.pprint('GREEN', 'Fine - liblua version %s.%s.%s found' % (major, minor, point))
					else:
						checks_ok = false
						env.pprint('RED', 'liblua > 5.0 (optional) not found')

		if checks_ok:
			env['CXXFLAGS_LUA']= ['-DHAVE_LUA']
			env['LINKFLAGS_LUA'] = ['-llua', '-llualib']

		env['CACHED_LUA'] = 1;
		env = conf.Finish()
		opts.Save(optionFile, env)
