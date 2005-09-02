#! /usr/bin/env python

"""
help       -> scons -h
compile    -> scons
clean      -> scons -c
install    -> scons install
uninstall  -> scons -c install
configure  -> scons configure prefix=/tmp/ita debug=full extraincludes=/usr/local/include:/tmp/include prefix=/usr/local

Run from a subdirectory -> scons -u
The variables are saved automatically after the first run (look at cache/kde.cache.py, ..)
"""

# how to do ?

# tar xjvf bksys/scons-mini.tar.bz2
# ./scons

# Suggested plan ? (ita)
# * convert some kde4 code as a base for further developments
# * find the variables for the configuration modules
# * write the configuration modules
# * use the automatic converter for the rest of the tree

###################################################################
# LOAD THE ENVIRONMENT AND SET UP THE TOOLS
###################################################################

## Load the builders in config
env = Environment( tools=['default', 'generic', 'qt4'], toolpath=['./', './bksys'])

#env.KDEuse("environ rpath")
#env.KDEuse("environ rpath lang_qt thread nohelp")

## TODO this is to be moved to modules

# warning: parameters are lists
env['LIB_QT']='QtOpenGL_debug QtGui_debug png SM ICE Xi Xrender Xrandr Xcursor Xinerama freetype fontconfig Xext X11 m QtCore_debug z dl GLU GL pthread'.split()

env['LIB_QT']         = []
env['LIB_QTCORE']     = []
env['LIB_QT3SUPPORT'] = []

# prefix/lib and prefix/lib/kde4 (ita)
env['KDE_RPATH']= [env.join(env['PREFIX'], 'lib'), env.join(env['PREFIX'], 'lib', 'kde4')]

# TODO: we need a config.h and i don't have time to use the one from elsewhere (i know some project does it)
import os
if not os.path.exists('build/config.h'):
	dest=open('build/config.h', 'w')
	dest.write('#define foo bar\n')
	dest.close()

env.Append(CPPFLAGS = ['-Ibuild'])

###################################################################
# SCRIPTS FOR BUILDING THE TARGETS
###################################################################

## BuilDir example - try to have all sources to process in only one top-level folder
SetOption('duplicate', 'soft-copy')
#env.BuildDir('build', '.', duplicate=0)

subdirs="""
dcop
libltdl
kdefx
kdecore
kunittest
kdeui
"""

env['_BUILDDIR_']='build'
for dir in subdirs.split():
	env.BuildDir( env.join('#build', dir), dir, duplicate=0)
	# TODO: one dir at a time but later the following line will be uncommented
	#env.SConscript( env.join('dir', 'SConscript') )

env.subdirs('build/dcop/')

#env.SConscript( [env.join('build', 'dcop', 'SConscript')] )

###################################################################
# CONVENIENCE FUNCTIONS TO EMULATE 'make dist' and 'make distclean'
###################################################################

# if you forget to add a version number, the one in the file VERSION will be used instead
env.dist('kdelibs', '4.0.0')

