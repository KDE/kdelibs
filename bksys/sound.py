#! /usr/bin/env python

BOLD   ="\033[1m"
RED    ="\033[91m"
GREEN  ="\033[92m"
YELLOW ="\033[93m"
CYAN   ="\033[96m"
NORMAL ="\033[0m"

def exists(env):
	return true

def generate(env):
	import SCons.Util, os

	if env['help']:
		print """
"""+BOLD+"""*** Sound options ***
-----------------------"""+NORMAL+"""
"""+BOLD+"""* noalsa  """+NORMAL+""": disable alsa
"""+BOLD+"""* nojack """+NORMAL+""": disable jack
ie: """+BOLD+"""scons configure noalsa=1 nojack=1
"""+NORMAL

    # we need this module
    pkgs = Tool('pkgconfig', ['./bksys'])
	pkgs.generate(env)

	if 'configure' in env['TARGS'] or not env.has_key('ISCONFIGURED') or not os.path.isfile('config.h'):

		import sys
		if 'noalsa' in env['TARGS']:
			print "-> Alsa module disabled by user"
			haveAlsa = 0
		else:
			haveAlsa = env.pkgConfig_findPackage('ALSA', 'alsa','1.0')

		if 'nojack' in env['TARGS']:
			print "-> Jack module disabled by user"
			haveJack = 0
		else:
			haveJack = env.pkgConfig_findPackage('JACK', 'jack', '0.77')

		haveLadspa  = conf.CheckHeader('ladspa.h')
		haveLiblrdf = conf.CheckLibWithHeader('lrdf', ['stdio.h', 'lrdf.h'], 'C', 'lrdf_init();')
		
		haveLiblo   = env.pkgConfig_findPackage('LIBLO', 'liblo', '0.7')
		haveLibmad  = env.pkgConfig_findPackage('MAD', 'mad', '0.10')
		haveLibdssi = env.pkgConfig_findPackage('DSSI', 'dssi', '0.4')

		if haveAlsa:
			env.Append(SOUND_CCFLAGS = '-DHAVE_ALSA')
			#os.popen('echo "#define HAVE_ALSA">>config.h')
		if haveJack:
			env.Append(SOUND_CCFLAGS = '-DHAVE_LIBJACK')
			#os.popen('echo "#define HAVE_LIBJACK">>config.h')
		if haveLadspa:
			env.Append(SOUND_CCFLAGS = '-DHAVE_LADSPA')
			#os.popen('echo "#define HAVE_LADSPA">>config.h')
		if haveLiblo:
			env.Append(SOUND_CCFLAGS = '-DHAVE_LIBLO')
			#os.popen('echo "#define HAVE_LIBLO">>config.h')
		if haveLibmad:
			env.Append(SOUND_CCFLAGS = '-DHAVE_LIBMAD')
			#os.popen('echo "#define HAVE_LIBMAD">>config.h')
		if haveLiblrdf:
			env.Append(SOUND_CCFLAGS = '-DHAVE_LIBLRDF')
			env.AppendUnique(SOUND_LDFLAGS = '-llrdf')
			#os.popen('echo "#define HAVE_LIBLRDF">>config.h')

		opts.Save(cachefile, env)

	if env.has_key('SOUND_CCFLAGS'):
		env.AppendUnique(CCFLAGS = env['SOUND_CCFLAGS'] )
		env.AppendUnique(CXXFLAGS = env['SOUND_CCFLAGS'] )
	if env.has_key('SOUND_LDFLAGS'):
		env.AppendUnique(LINKFLAGS = env['SOUND_LDFLAGS'] )

