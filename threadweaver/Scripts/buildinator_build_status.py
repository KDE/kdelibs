# a class that holds the build status (aka results of a build run), an object
# of this class represents a record in the build status database later:

class BuildStatus:
    """BuildStatus collects information about a build+test run"""
    def dumpBuildStatus( self ):
        CheckoutStatusText = ( 'not performed', 'success', 'failure' )
        ConfigureStatusText = ( 'not performed', 'success', 'failure' )
        CompileStatusText = ( 'not performed', 'success', 'success (warnings)', 'failure' )
        InitialTestStatusText = ( 'not performed', 'success', 'tests failed', 'test crashed', 'tests timed out' )
        """Print the build status to the console tersely"""
        print 'build result for revision ' + str(self.revision) + ', committed by ' + self.committer + ':'
        print '--> build platform: ' + self.platform
        print '--> configuration: "' + self.prefix + '", configured with options "' + self.options + '"'
        print '--> checkout status: ' + CheckoutStatusText[self.checkoutStatus]
        print '--> configure status: ' + ConfigureStatusText[self.configureStatus]
        print '--> compile status: ' + CompileStatusText[self.compileStatus]
        print '--> initial test status: ' + InitialTestStatusText[self.initialTestStatus]
        
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
    initialTestStatus = 0
    # committer of the revision
    committer = ''
    

