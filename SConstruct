#! /usr/bin/env python

"""
help       -> scons -h
compile    -> scons
clean      -> scons -c
install    -> scons install
uninstall  -> scons -c install
configure  -> scons configure debug=full extraincludes=/usr/local/include:/tmp/include prefix=/usr/local

Run from a subdirectory -> scons -u (or export SCONSFLAGS="-u" in your ~/.bashrc)

Documentation is in bksys/design
"""

# QUICKSTART (or look in bksys/design)
# cd kdelibs
# tar xjvf bksys/scons-mini.tar.bz2
# ./scons

# WARNING
# Nothing will work out of the box until the config.h system is ready...

# Suggested plan ? (ita)
# * convert some kde4 code as a base for further developments : ok
# * find the variables for the configuration modules          : ok
# * create the config.h system                                : doing it at the moment
# * write the configuration modules                           : __
# * use the automatic converter for the rest of the tree      : __

###################################################################
# LOAD THE ENVIRONMENT AND SET UP THE TOOLS
###################################################################

## We assume that 'bksys' is our admin directory
import sys
sys.path.append('bksys')

## Import the main configuration tool
from generic import configure
config = {
	'modules'  : 'generic lowlevel libxml qt4 kde4',
	'builddir' : 'build',
	'config.h' : 1,
	'rpath'    : 1,
}

# and the config.h
env=configure(config)

# now the kdemacros (TODO a bootstrap module ?)
dest = open(env.join('build','kdemacros.h'), 'w')
dest.write('#include <kdemacros.h.in>\n')
dest.close()

###################################################################
# SCRIPTS FOR BUILDING THE TARGETS
###################################################################

subdirs="""
dcop
libltdl
kdefx
mimetypes
"""
# kdecore kunittest kdeui

# TODO this will not stay like this ..
dirs=[] # the dirs to process are appended to this var in the loop below
for dir in subdirs.split():
	env.BuildDir( env.join('#build', dir), dir, duplicate=0)
	dirs.append( env.join('#build', dir) )
env.subdirs(dirs)

###################################################################
# CONVENIENCE FUNCTIONS TO EMULATE 'make dist' and 'make distclean'
###################################################################

# if you forget to add a version number, the one in the file VERSION will be used instead
env.dist('kdelibs', '4.0.0')

