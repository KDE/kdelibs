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

# the entry point for the whole build test process:
def ExecuteBuildAndTest ( WorkDir, Revision, Module, ProFileName):
    """This function runs the sequence of checking out, building and testing a revision"""
    SrcDir = WorkDir+ "/Src"
    BuildDir = WorkDir + "/Build"
    LogDir = WorkDir + "/Logs"
    MinimumTestRevision = -1

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

    # a dictionary with the configuration name and qmake options:
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

    print "Cleaning up (deleting all in SrcDir and BuildDir, leave Logs) ..."
    CleanUp (SrcDir, BuildDir)
    print "Done, results in " + LogDir

    # FIXME: return something useful

def GetSvnLog ( SvnUrl ):
    """Retrieve the subversion log for a subversion URL and return it in a list."""
    Cmd = "svn log " + SvnUrl
    File = os.popen (Cmd, 'r')
    return File.readlines()

def GetRevisionList( Lines ):
    """Make an integer list of revisions, sorted lowest-to-highest"""
    RevisionLine = re.compile( '^r\d+\s')
    Revisions = []
    for Line in Lines:
        Match = RevisionLine.match( Line )
        if Match:
            Revision = Match.group().rstrip()[1:]
            Revisions = Revisions + [ int(Revision) ]
    Revisions.sort()
    return Revisions

