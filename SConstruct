#! /usr/bin/env python

import os, sys

"""
help       -> scons -h
compile    -> scons
clean      -> scons -c
install    -> scons install
uninstall  -> scons -c install
configure  -> scons configure debug=full extraincludes=/usr/local/include:/tmp/include prefix=/usr/local

Run from a subdirectory -> scons -u
The variables are saved automatically after the first run (look at cache/kde.cache.py, ..)
"""

# How to do ?

# cd kdelibs
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
env = Environment( tools=['generic', 'libxml', 'qt4'], toolpath=['./bksys'],
		   ENV={'PATH' : os.environ['PATH']})

if env['HELP']:
	sys.exit(0)

## Add the builddir as an include path for everyone
env.Append(CPPFLAGS = ['-Ibuild'])

## TODO this is to be moved to modules
##############  -- IMPORTANT --
## warning: the naming matters, see genobj -> uselib in generic.py
## warning: parameters are lists
## obj.uselib='Z PNG KDE4' will add all the flags
##   and paths from zlib, libpng and kde4 vars defined below

########## zlib
env['LIB_Z']               = ['z']

########## png
env['LIB_PNG']             = ['png', 'z', 'm']

########## sm
env['LIB_SM']              = ['SM', 'ICE']

########## X11
env['LIB_X11']             = ['X11']
env['LIBPATH_X11']         = ['/usr/X11R6/lib/']

########## QT
# QTLIBPATH is a special var used in the qt4 module - has to be changed (ita)
env['LIBPATH_QT']          = env['LIBPATH_X11']+[env['QTLIBPATH']]
env['LIB_QT']              = ['QtGui_debug', 'pthread', 'Xext']+env['LIB_Z']+env['LIB_PNG']+env['LIB_X11']+env['LIB_SM']

## QT3SUPPORT
env['LIB_QT3SUPPORT']      = ['Qt3Support_debug']
env['CXXFLAGS_QT3SUPPORT'] = ['-DQT3_SUPPORT']

env['LIB_QTCORE']          = ['QtCore_debug']
env['LIB_QTDESIGNER']      = ['QtDesigner_debug']
env['LIB_QTGUI']           = ['QtGui_debug']
env['LIB_QTNETWORK']       = ['QtNetwork_debug']
env['LIB_QTOPENGL']        = ['QtOpenGL_debug']
env['LIB_QTSQL']           = ['QtSql_debug']
env['LIB_QTXML']           = ['QtXml_debug']

########### KDE4
env['LIBPATH_KDE4']        = env['LIBPATH_QT']

## not very portable but that's the best i have at the moment (ITA)
includes=['.','dcop','kio','kio/kio','kio/kfile','kdeui','kdecore','libltdl','kdefx']
env['INCLUDES_KDE4']=['#build']
for dir in includes:
	env['INCLUDES_KDE4'].append('#'+dir)
	env['INCLUDES_KDE4'].append('#build/'+dir)

# prefix/lib and prefix/lib/kde4 (ita)
env['RPATH_KDE4']= [
	env['QTLIBPATH'], env.join(env['PREFIX'], 'lib'), env.join(env['PREFIX'], 'lib', 'kde4')
]

#######################################
## install paths
env['KDEBIN']='/usr/bin'
env['KDEDATA']='/usr/share'
env['KDEMIME']='/usr/share/mimelnk'

#######################################
## other stuff
env['CONVENIENCE']         = ['-fPIC','-DPIC'] # TODO flags for convenience libraries

# TODO: we need a config.h and i don't have time to use the one from elsewhere (i know some project does it)
# look at dcop/SConscript for how to build a .h from a python function cleanly
import os
if not os.path.exists('build/config.h'):
	os.mkdir('build')
	dest=open('build/config.h', 'w')
	dest.close()

if not os.path.exists('build/kdemacros.h'):
	dest = open('build/kdemacros.h', 'w')
	dest.write('#include <kdemacros.h.in>\n')
	dest.close()

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
mimetypes
"""

env['_BUILDDIR_']='build'
for dir in subdirs.split():
	env.BuildDir( env.join('#build', dir), dir, duplicate=0)
	# TODO: one dir at a time but later the following line will be uncommented
	#env.SConscript( env.join('dir', 'SConscript') )

env.subdirs('build/dcop/ build/mimetypes')
#env.SConscript( [env.join('build', 'dcop', 'SConscript')] )

###################################################################
# CONVENIENCE FUNCTIONS TO EMULATE 'make dist' and 'make distclean'
###################################################################

# if you forget to add a version number, the one in the file VERSION will be used instead
env.dist('kdelibs', '4.0.0')

