# Copyright	David Faure	<faure@kde.org>	2005
# BSD license (see COPYING)

def	detect(env):
	import os, re

	if env['CC'] !=	'gcc' and env['CC']	!= 'cl':
		return False
	
	error_regexp = re.compile('Usage|ERROR|unrecognized	option|unknown option|WARNING|missing|###')
	unused_regexp =	re.compile('Reading	specs|Using	built|Configured with|Thread model|Copyright|###')
	
	for	flag in	['-v', '-V', '--version', '-version', '/help']:
		# should I use context.TryAction here?
		syspf =	os.popen( env['CC']+' '+flag+' 2>&1' )
		for	line in	syspf.read().split( '\n' ):
			if re.search( error_regexp,	line, 'I' ):
				continue
			if re.search( unused_regexp, line, 'I' ):
				continue
			if len(line):
				return line

	return False
