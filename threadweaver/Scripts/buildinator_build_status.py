# a class that holds the build status (aka results of a build run), an object
# of this class represents a record in the build status database later:

class BuildStatus (dict):
    """BuildStatus collects information about a build+test run"""
    def  __init__ (self, Status= None):
        """ Construct an objct, possibly copying it"""
        if Status is not None:
            self.svnUrl = Status.svnUrl
            self.projectFile = Status.projectFile
            self.revision = Status.revision
            self.platform = Status.platform
            self.options = Status.options
            self.prefix = Status.prefix
            self.checkoutStatus = Status.checkoutStatus
            self.configureStatus = Status.configureStatus
            self.compileStatus = Status.compileStatus
            self.overallTestStatus = Status.overallTestStatus
            self.committer = Status.committer
            self.numberOfTestRuns = Status.numberOfTestRuns
            self.testResults = Status.testResults
            
    def dumpBuildStatus( self ):
        CheckoutStatusText = ( 'not performed', 'success', 'failure' )
        ConfigureStatusText = ( 'not performed', 'success', 'failure' )
        CompileStatusText = ( 'not performed', 'success', 'success (warnings)', 'failure' )
        InitialTestStatusText = ( 'not performed', 'success', 'tests failed', 'test crashed', 'tests timed out' )
        TestsGood = 0
        for Run in self.testResults:
            if Run == 1: # success
                TestsGood += 1
        TestsTotal = len(self.testResults)
        TestsBad = TestsTotal - TestsGood
        TestResultsString = '[' + str(TestsGood) + '/' + str(TestsBad) + '/' + str(TestsTotal) + ']'

        """Print the build status to the console tersely"""
        print 'build result for revision ' + str(self.revision) + ', committed by ' + self.committer + ':'
        print '--> build platform: ' + self.platform
        print '--> Subversion URL: ' + self.svnUrl
        print '--> configuration: "' + self.prefix + '", configured with options "' + self.options + '"'
        print '--> checkout status: ' + CheckoutStatusText[self.checkoutStatus]
        print '--> configure status: ' + ConfigureStatusText[self.configureStatus]
        print '--> compile status: ' + CompileStatusText[self.compileStatus]
        print '--> overall test status: ' + InitialTestStatusText[self.overallTestStatus] + ' ' + TestResultsString

    def hasErrors( self ):
        """Returns whether errors occured during the build process"""
        if self.checkoutStatus == 2 or self.configureStatus == 2 or self.compileStatus == 3 \
               or self.overallTestStatus != 1:
            return True
        else:
            return False
        

    # the svn URL this is working on:
    svnUrl = ''
    # the project file to use for creating makefiles
    # FIXME: this may need to be subclassed for different makefile generators
    projectFile = ''
    # the revision that was build in this run:
    revision = 0
    # the platform that this was built on:
    platform = ''
    # the build options used:
    options = ''
    # the prefix (usually Debug or Release):
    prefix = ''
    # check-out status (0 = not performed, 1 = success, 2 = failed)
    checkoutStatus = 0
    # configure status (0 = not performed, 1 = success, 2 = failed)
    configureStatus = 0
    # compilestatus (0 = not performed, 1 = success, 2 = success with warnings, 3 = failed)
    compileStatus = 0
    # first test run status ( 0 = not performed, 1 = success, 2 = test failed, 3 = crash, 4 = timeout)
    overallTestStatus = 0
    # number of times the tests are supposed to be run:
    numberOfTestRuns = 1
    # list of test run results:
    testResults = []
    # committer of the revision
    committer = ''

        

    

