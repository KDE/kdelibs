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

###################################################################
# LOAD THE ENVIRONMENT AND SET UP THE TOOLS
###################################################################

## Load the builders in config
env = Environment( tools=['default', 'generic', 'qt4'], toolpath=['./', './bksys'])

#env.KDEuse("environ rpath")
#env.KDEuse("environ rpath lang_qt thread nohelp")

###################################################################
# SCRIPTS FOR BUILDING THE TARGETS
###################################################################

## BuilDir example - try to have all sources to process in only one top-level folder
SetOption('duplicate', 'soft-copy')
env.BuildDir('build', '.', duplicate=0)
#env.BuildDir('#build/kde3_examples', '#kde3_examples')
#env.subdirs('build/kde3_examples')

## Normal build
#env.subdirs('kde3_examples')

###################################################################
# CONVENIENCE FUNCTIONS TO EMULATE 'make dist' and 'make distclean'
###################################################################

## If your app name and version number are defined in 'version.h', use this instead:
## (contributed by Dennis Schridde devurandom@gmx@net)
#import re
#INFO = dict( re.findall( '(?m)^#define\s+(\w+)\s+(.*)(?<=\S)', open(r"version.h","rb").read() ) )
#APPNAME = INFO['APPNAME']
#VERSION = INFO['VERSION']

# if you forget to add a version number, the one in the file VERSION will be used instead
#env.dist('bksys', '1.5.1')

#env.dist('bksys')

