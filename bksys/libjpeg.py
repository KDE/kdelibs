# Copyright Thomas Nagy 2005
# Copyright Ralf Habacker 2005
# BSD license (see COPYING)
def exists(env):
	return true

def generate(env):

	def Check_libjpeg(context):
		import SCons.Util
		ret = 0

		if env['WINDOWS']:
			context.Message('Checking for libjpeg... ')
			qtdir = os.getenv("QTDIR")
			if qtdir: 
				# TODO: check if libjpeg is really in QTCore lib 
				env['CPPPATH_JPEG'] = [qtdir + "\\src\\3rdparty\\libjpeg"]
				env['LIBPATH_JPEG'] = [qtdir + "\\lib"]
				env['LIB_JPEG'] = ['QtCore4'] # TODO: check if debug version is used 
				env['CACHED_JPEG'] = 1
				ret = 1
			context.Result(ret)
		else:
			#we need to check for libjpeg6b then normal libjpeg to
			#be compatible with the current autoconf check
			env['CACHED_JPEG'] = 0
			have_jpeg_headers = env.find_file('jpeglib.h',['/usr/include','/usr/local/include'] + env['EXTRAINCLUDES'])
			have_jpeg_6b = conf.CheckLib('jpeg6b')
			have_normal_jpeg = conf.CheckLib('jpeg')
			context.Message('Checking for libjpeg... ')
			if (have_jpeg_6b or have_normal_jpeg) and have_jpeg_headers:
				env['LINKFLAGS_JPEG'] = '-ljpeg'
				env['CACHED_JPEG'] = 1
			
			dest=open(env.join(env['_BUILDDIR_'], 'config-jpeg.h'), 'w')
			dest.write('/* libjpeg configuration created by bksys */\n')

			if env['CACHED_JPEG']:
				dest.write('#define HAVE_LIBJPEG 1\n');
				ret = 1
		
			context.Result(ret)
		return ret

	from SCons.Options import Options
	import os

	optionFile = env['CACHEDIR'] + 'libjpeg.cache.py'
	opts = Options(optionFile)
	opts.AddOptions(
		('CACHED_JPEG', 'Whether libjpeg is available'),
		('CXXFLAGS_JPEG',''),
		('CCFLAGS_JPEG',''),
		('LINKFLAGS_JPEG',''),
		('CPPPATH_JPEG',''),
		('LIB_JPEG',''),
		('LIBPATH_JPEG',''),
		)
	opts.Update(env)
	opts.Save(optionFile, env)

	if not env['HELP'] and (env['_CONFIGURE_'] or not env.has_key('CACHED_JPEG')):
		conf = env.Configure(custom_tests =	 { 'Check_libjpeg' : Check_libjpeg} )
		if not conf.Check_libjpeg():
			print 'libjpeg not found (mandatory).'
			
		env = conf.Finish()
		opts.Save(optionFile, env)
	
