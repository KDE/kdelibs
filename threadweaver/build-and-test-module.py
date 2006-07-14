#!/usr/bin/python

import re
import os
import sys
import time
import platform

def CheckoutSubversionRevision (Module, Revision, SrcDir, LogDir):
    """Checks a certain revision of threadweaver out of KDESVN into SrcDir"""
    Cmd="svn co -r" + str(Revision) + ' ' + Module + ' ' \
         + SrcDir + ' ' \
         + "> " + LogDir + "/SubversionCheckout.log 2>&1"
    # print Cmd
    ReturnCode = 0
    try:
        ReturnCode = os.system (Cmd)
    except:
        pass
    
    if ReturnCode != 0:
        raise "Cannot check out revision " + str(Revision)

def CleanUp (SrcDir, BuildDir):
    """Delete temporary src and build directories"""
    os.system ("rm -Rf " + SrcDir)
    os.system ("rm -Rf " + BuildDir)

def Build (SrcDir, BuildDir, Prefix, Options, ProFileName, LogDir):
    """Build the sources in SrcDir in BuildDir/Prefix using Options for qmake"""
    BuildPath = BuildDir + "/" + Prefix
    # if exceptions occur, stop the script (no catching):
    os.system("mkdir -p " + BuildPath)
    os.system("(cd " + BuildPath + " && qmake " \
              + Options + " " + SrcDir + '/' + ProFileName + ')' \
              + " > " + LogDir + "/qmake-" + Prefix + ".log 2>&1")
    return os.system("(cd " + BuildPath + " && make all " \
                     + " > " + LogDir + "/make-all-" + Prefix + ".log 2>&1 )")
    
def Test (BuildDir, Prefix, LogDir):
    """Run the unit tests in BuildDir/Prefix"""
    BuildPath = BuildDir + "/" + Prefix
    return os.system("(cd " + BuildPath + " && make test " \
                     + " > " + LogDir + "/make-test-" + Prefix + ".log 2>&1 )")

# "main":

# parse command line args to find the revision to test:
print """Build and test a subversion revision of a module. (C) Mirko Boehm, 2006
This script is licensed as a part of the ThreadWeaver multithreading suite
under the LGPL.
Run this script in an empty directory.
"""

SrcDir = os.getcwd() + "/Src"
BuildDir = os.getcwd() + "/Build"
LogDir = os.getcwd() + "/Logs"
Module = ''
ProFileName = ''

Revision = 0
MinimumTestRevision = -1
try:
    Revision = int (sys.argv[1])
    Module = sys.argv[2]
    ProFileName = sys.argv[3]
except:
    print "Usage: " + sys.argv[0] + " <SVN revision to test>"
    print "Example: " + sys.argv[0] \
          + ' 452230 svn+ssh://svn.kde.org/home/kde/trunk/kdenonbeta/threadweaver ' \
          + 'ThreadWeaver.pro'
    sys.exit (-1)
else:
    if (Revision < 452230):
        raise ("Only revisions starting at 452230 are supported (Qt4 based versions)")

    print "Module:   " + Module
    print "Project:  " + ProFileName
    print "SrcDir:   " + SrcDir
    print "BuildDir: " + BuildDir
    print "LogDir:   " + LogDir
    print
    
    try:
        for Directory in [ SrcDir, BuildDir, LogDir]: 
            os.system ("mkdir -p " + Directory)
        print "Clearing existing logs in " + LogDir
        os.system("rm -f " + LogDir + "/*")
    except:
        print "Cannot make src or build directory. Check permissions."
        sys.exit (-2)

    print "Checking out revision " + str(Revision)
    CheckoutSubversionRevision (Module, Revision, SrcDir, LogDir)
    Options = { "Debug" : "CONFIG+=debug",
                "Release" : "CONFIG+=release" }
    for Prefix, Option in Options.items():
        print 'Building with options "' + Option + '" in ' + Prefix
        if Build (SrcDir, BuildDir, Prefix, Option, ProFileName, LogDir) != 0:
            print "Build failed, no testing"
            continue
        print "Build successful"
        if Revision < MinimumTestRevision:
            print "Not testing, no unit test in revisions below " + MinimumTestRevision
            continue
        print "Executing initial unit test run"
        if Test (BuildDir, Prefix, LogDir) == 0:
            print "Tests successful"
            # run multiple tests
        else:
            print "Tests failed!"
            
    print "Cleaning up..."
    CleanUp (SrcDir, BuildDir)
    print "Done, results in " + LogDir
