#! /usr/bin/env python

Import('env')

env.subdirs('iceauth')

install_headers = """
include/kdelibs_export_win.h
"""

env.bksys_insttype( 'KDEINCLUDE', '', install_headers )

# TODO: scan dynamic 
if env['CC'] == '_gcc':

	inst_headers = """
	include/mingw/fcntl.h
	include/mingw/grp.h
	include/mingw/ifaddrs.h
	include/mingw/netdb.h
	include/mingw/pwd.h
	include/mingw/readdir.h
	include/mingw/signal.h
	include/mingw/stdio.h
	include/mingw/stdlib.h
	include/mingw/string.h
	include/mingw/time.h
	include/mingw/unistd.h
	"""
	env.bksys_insttype( 'KDEINCLUDE', 'mingw', inst_headers )

	sys_headers = """
	include/mingw/sys/ioctl.h
	include/mingw/sys/mman.h
	include/mingw/sys/resource.h
	include/mingw/sys/select.h
	include/mingw/sys/signal.h
	include/mingw/sys/socket.h
	include/mingw/sys/stat.h
	include/mingw/sys/time.h
	include/mingw/sys/types.h
	include/mingw/sys/uio.h
	include/mingw/sys/un.h
	include/mingw/sys/unistd.h
	include/mingw/sys/utsname.h
	include/mingw/sys/wait.h
	"""
	env.bksys_insttype( 'KDEINCLUDE', 'mingw\\sys', sys_headers )
	
	arpa_headers = """
	include/mingw/arpa/inet.h
	"""
	env.bksys_insttype( 'KDEINCLUDE', 'mingw\\arpa', arpa_header )

	asm_headers = """
	include/mingw/asm/byteorder.h
	"""
	env.bksys_insttype( 'KDEINCLUDE', 'mingw\\asm', asm_header )

	netinet_headers = """
	include/mingw/netinet/in.h
	include/mingw/netinet/tcp.h
	"""
	env.bksys_insttype( 'KDEINCLUDE', 'mingw\\netinet', netinet_header )
	
	net_headers = """
	include/mingw/net/if.h
	"""
	env.bksys_insttype( 'KDEINCLUDE', 'mingw\\net', net_header )
