#!/usr/bin/python

import os
import sys
import buildinator_common

# main
print """Build and test all subversion revisions of a module. (C) Mirko Boehm, 2006
This script is licensed as a part of the ThreadWeaver multithreading suite
under the LGPL.

This is the massive build and unit test script. It will iteratively
retrieve all Subversion revisions of ThreadWeaver (starting from a certain
known one at which "make test" in the toplevel Makefile worked for the first
time), build it for debug and release targets and run the unit tests 100 times
each. It will then report a summary. All the build and source files will be
deleted. When the script completes, it will leave a folder for every
Subversion revision that contains the Logs of the executed processes.
"""

MinimumRevision = -1
#    if (Revision < 452230):
#        raise ("Only revisions starting at 452230 are supported (Qt4 based versions)")

# get Subversion Url from command line arg 1:
Module = ''
ProFileName = ''

# here we break (required arguments):
try:
    Module = sys.argv[1]
    ProFileName = sys.argv[2]
    MinimumRevision = int (sys.argv[3])
except:
    print 'Usage: ' + sys.argv[0] + ' <SvnUrl> <ProFileName> <minimum SVN revision to test>'
    sys.exit( -1 )

print '--> Minimum revision is ' + str(MinimumRevision)

Lines = buildinator_common.GetSvnLog( Module )
Revisions = buildinator_common.GetRevisionList( Lines )

# weed out revision below MinimumRevision:
def newer_than_MinimumRevision(n):
    return n >= MinimumRevision

Revisions = filter ( newer_than_MinimumRevision, Revisions)

print '--> Autobuilding ' + str(len(Revisions)) + ' revisions'

# start building:
for Revision in Revisions:
    print '--> Autobuilding #' + str (Revision)
    WorkFolder = str(Revision)
    os.system( 'mkdir -p ' + WorkFolder )
    WorkDir = os.getcwd() + '/' + WorkFolder
    buildinator_common.ExecuteBuildAndTest ( WorkDir, Revision, Module, ProFileName)

