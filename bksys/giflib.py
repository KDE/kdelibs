# Copyright Benjamin Reed 2005
# BSD license (see COPYING)
def exists(env):
	return true

def generate(env):

	def Check_giflib(context):
		import SCons.Util

		gifsource = """
#include <gif_lib.h>

int main(int argc, char **argv) {
	if (GIF_OK != 1) {
		return 1;
	}
	return 0;
}
"""

		env['CACHED_GIFLIB'] = 0
		lastLIBS = env['LIBS']

		context.Message('Checking for giflib ... ')
		for lib in [ 'ungif', 'gif' ]:

			env.Replace(LIBS = [lib])

			ret = conf.TryLink(gifsource, '.c')
			if ret:
				env['CXXFLAGS_GIFLIB'] = ['-DHAVE_GIFLIB']
				env['LINKFLAGS_GIFLIB'] = ['-l' + lib]
				env['CACHED_GIFLIB'] = ret
				break

		env.Replace(LIBS = lastLIBS)
		context.Result(env['CACHED_GIFLIB'])
		return env['CACHED_GIFLIB']

	from SCons.Options import Options
	import os

	optionFile = env['CACHEDIR'] + 'giflib.cache.py'
	opts = Options(optionFile)
	opts.AddOptions(
		('CACHED_GIFLIB', 'Whether the GIF library is available'),
		('CXXFLAGS_GIFLIB',''),
		('LINKFLAGS_GIFLIB',''),
		)
	opts.Update(env)

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('CACHED_GIFLIB')):
		conf = env.Configure( custom_tests = { 'Check_giflib' : Check_giflib } )
		if not conf.Check_giflib():
			print 'giflib not found.'
		else:
			print 'using ' + env['LINKFLAGS_GIFLIB'][0] + ' to link giflib'

		env = conf.Finish()

		opts.Save(optionFile, env)

