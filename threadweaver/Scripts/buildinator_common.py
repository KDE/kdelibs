import re
import os
import sys
import time
import platform
from buildinator_build_status import BuildStatus

def CheckoutSubversionRevision (Status, SrcDir, LogDir):
    """Checks a certain revision of threadweaver out of KDESVN into SrcDir"""
    Cmd="svn co -r" + str(Status.revision) + ' ' + Status.svnUrl + ' ' \
         + SrcDir + ' ' \
         + "> " + LogDir + "/SubversionCheckout.log 2>&1"
    # print Cmd
    # print "Checking out revision " + str(Status.revision)
    ReturnCode = 0
    try:
        ReturnCode = os.system (Cmd)
        Status.checkoutStatus = 1 # success
    except:
        Status.checkoutStatus = 2 # failed
    return Status

def CleanUp (Status, SrcDir, BuildDir):
    """Delete temporary src and build directories"""
    # build status is ignored here, but we may want to skip cleanup if some step of the build failed
    os.system ("rm -Rf " + SrcDir)
    os.system ("rm -Rf " + BuildDir)
    return Status

def Build (Status, SrcDir, BuildDir, LogDir):
    """Build the sources in SrcDir in BuildDir/Prefix using Options for qmake"""
    BuildPath = BuildDir + "/" + Status.prefix
    # if exceptions occur, stop the script (no catching):
    os.system("mkdir -p " + BuildPath)
    rc = os.system("(cd " + BuildPath + " && qmake " \
                   + Status.options + " " + SrcDir + '/' + Status.projectFile + ')' \
                   + " > " + LogDir + "/qmake-" + Status.prefix + ".log 2>&1")
    if rc == 0:
        Status.configureStatus = 1 # success
    else:
        Status.configureStatus = 2 # failure
    rc = os.system("(cd " + BuildPath + " && make all " \
                   + " > " + LogDir + "/make-all-" + Status.prefix + ".log 2>&1 )")
    # FIXME: find out about "success with warnings"
    if rc == 0:
        Status.compileStatus = 1 # success
    else:
        Status.compileStatus = 3 # failure
    return Status
    
def Test (Status, BuildDir, Prefix, LogDir):
    """Run the unit tests in BuildDir/Prefix"""
    BuildPath = BuildDir + "/" + Prefix
    Cmd = "(cd " + BuildPath + " && make test " \
          + " > " + LogDir + "/make-test-" + Prefix + ".log 2>&1 )"
    Status.overallTestStatus = 1 # success
    for Step in range( Status.numberOfTestRuns ):
        rc =os.system(Cmd)
        if rc == 0:
            Status.testResults = Status.testResults + [ 1 ]
        else:
            Status.testResults = Status.testResults + [ 2 ]
            Status.overallTestStatus = 2 # failed
    return Status

# the entry point for the whole build test process:
def ExecuteBuildAndTest ( Status, WorkDir ):
    """This function runs the sequence of checking out, building and testing a revision"""
    SrcDir = WorkDir+ "/Src"
    BuildDir = WorkDir + "/Build"
    LogDir = WorkDir + "/Logs"
    MinimumTestRevision = -1
    Errors = False
    # common object, will be copied for the different runs
    Status.platform = platform.platform()
    BuildResults = [] # will hold a list of BuildStatus objects,one for every configuration
    # print "SrcDir:   " + SrcDir
    # print "BuildDir: " + BuildDir
    # print "LogDir:   " + LogDir
    # print
    
    try:
        for Directory in [ SrcDir, BuildDir, LogDir]: 
            os.system ("mkdir -p " + Directory)
        # print "Clearing existing logs in " + LogDir
        os.system("rm -f " + LogDir + "/*")
    except:
        print "--> error: cannot make src or build directory, check permissions!"
        sys.exit (-2)
        
    Status = CheckoutSubversionRevision ( Status, SrcDir, LogDir)
    if Status.checkoutStatus != 1:  # success
        return Status # return just this one object, since we cannot run the build without sources
    
    # a dictionary with the configuration name and qmake options:
    Options = { 'Debug' : 'CONFIG+=debug',
                'Release' : 'CONFIG+=release' }

    for Prefix, Option in Options.items():
        # copy status over from status of common steps:
        CurrentStatus = BuildStatus( Status )
        CurrentStatus.options = Option
        CurrentStatus.prefix = Prefix
        # print 'Building with options "' + Option + '" in ' + Prefix
        CurrentStatus = Build (CurrentStatus, SrcDir, BuildDir, LogDir)
        if  CurrentStatus.compileStatus == 1 or CurrentStatus.compileStatus == 2:
            # print "Build successful"
            if Status.revision < MinimumTestRevision:
                print "--> warning: not testing, no unit test in revisions below " + MinimumTestRevision
            else:
                # print "Executing initial unit test run"
                CurrentStatus = Test (CurrentStatus, BuildDir, Prefix, LogDir) 
                if CurrentStatus.overallTestStatus == 1:
                    # print "Tests successful"
                    pass
                else:
                    print "--> notice: tests failed!"
        else:
            print "--> error: build failed, no testing"
        if CurrentStatus.hasErrors(): Errors = True
        BuildResults = BuildResults + [ CurrentStatus ]
    # print "Cleaning up (deleting all in SrcDir and BuildDir, leave Logs) ..."
    if Errors:
        print '--> notice: not cleaning up because of previous errors'
    else:
        CleanUp (Status, SrcDir, BuildDir)
    return BuildResults

    # FIXME: return something useful

def GetSvnLog ( SvnUrl ):
    """Retrieve the subversion log for a subversion URL and return it in a list."""
    Cmd = "svn log " + SvnUrl
    File = os.popen (Cmd, 'r')
    return File.readlines()

def GetRevisionList( SvnUrl ):
    """Make an BuildStatus list of revisions, sorted latest-to-older"""
    Lines = GetSvnLog ( SvnUrl )
    RevisionLine = re.compile( '^r\d+\s')
    Revisions = []
    for Line in Lines:
        Match = RevisionLine.match( Line )
        if Match:
            Status = BuildStatus()
            Pieces = Line.split('|')
            Status.committer = Pieces[1].strip()
            Status.revision = int (Pieces[0].rstrip()[1:])
            Status.svnUrl = SvnUrl
            Revisions = Revisions + [ Status ]
    return Revisions

