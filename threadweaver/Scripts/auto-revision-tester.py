#!/usr/bin/python

import os
import sys
import buildinator_common
from buildinator_build_status import BuildStatus

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

print '--> Minimum revision: ' + str(MinimumRevision)
print '--> Module:           ' + Module
print '--> Project:          ' + ProFileName

Revisions = buildinator_common.GetRevisionList( Module )
Results = []

# weed out revision below MinimumRevision:
def newer_than_MinimumRevision(n):
    return n.revision >= MinimumRevision

Revisions = filter ( newer_than_MinimumRevision, Revisions)

print '--> Autobuilding ' + str(len(Revisions)) + ' revisions'

# start building:
for Revision in Revisions:
    print '--> Autobuilding #' + str (Revision.revision)
    WorkFolder = str(Revision.revision)
    os.system( 'mkdir -p ' + WorkFolder )
    WorkDir = os.getcwd() + '/' + WorkFolder
    Revision.projectFile = ProFileName
    Revision.numberOfTestRuns = 100
    Results = Results + buildinator_common.ExecuteBuildAndTest ( Revision, WorkDir )

for Result in Results:
    Result.dumpBuildStatus()
