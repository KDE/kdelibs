#! /usr/bin/env python

Import('env')

install_headers = """
include/kdelibs_export_win.h
"""

kdewin32_sources = """
 src/bootstrap.cpp
 src/dummy.cpp
 src/fcntl.c
 src/fsync.c
 src/getenv.c
 src/grp.c
 src/inet.c
 src/kde_file_win.c
 src/mmap.c
 src/net.c
 src/pwd.c
 src/readdir.c
 src/realpath.c
 src/resource.c
 src/signal.c
 src/stdlib.c
 src/string.c
 src/syslog.c
 src/time.c
 src/uname.c
 src/unistd.c
 src/win32_utils.c
 src/win32_utils2.cpp
"""


# libkdewin23 - library for windows support
obj = env.qt4obj('shlib',env)
#obj.vnum     = '6.0.2'
obj.target   = 'kdewin32'
obj.source   = kdewin32_sources
obj.uselib   = 'QT QTCORE QT3SUPPORT'
obj.ccflags  = '-DUNICODE -DWIN32_LEAN_AND_MEAN'
if env['CC'] == 'gcc': 
	obj.includes = 'include include/mingw'
else:
	obj.includes = 'include include/msvc'
obj.execute()


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
