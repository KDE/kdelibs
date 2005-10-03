# Copyright David Faure <faure@kde.org> 2005
# BSD license (see COPYING)

def detect(env):
	import os, re

        # TODO: convert xlC and KCC code from kdelibs/configure.in.in

        error_regexp = re.compile('Usage|ERROR|unrecognized option|unknown option|WARNING|missing|###')
	for flag in ['-v', '-V', '--version', '-version']:
		# should I use context.TryAction here?
		syspf = os.popen(env['CXX']+' '+flag+' 2>&1 | egrep -v "Reading specs|Using built-?in specs|Configured with|Thread model"')
                # Take the first line, and check that it doesn't look like an error message.
		compiler_version=syspf.read().split('\n')[0]
		if len(compiler_version) and not re.search( error_regexp, compiler_version, 'I' ):
			return compiler_version

	return False
