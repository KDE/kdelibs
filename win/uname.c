/*
   This file is part of the KDE libraries
   Copyright (C) 2003-2004 Jaroslaw Staniek <js@iidea.pl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <windows.h>

#include <sys/utsname.h>
#include <stdio.h>

int kde_gethostname(char *__name, size_t __len);

/**
 MS Windows implementation of uname().

 E.g. on Windows 2000
 <code>
  utsname n;
  printf("%s, %s, %s, %s, %s\n", n.sysname, n.release, n.version, n.machine, n.nodename);
 </code>
 it will print something like: 
 <code>
  Microsoft Windows, 5.0 (2000), Dec 16 2004, i686, MYHOSTNAME
 </code>

 Note that utsname.version is just a compile time of kdewin32 library (__DATE__).
*/
KDEWIN32_EXPORT int uname(struct utsname *name)
{
	OSVERSIONINFO versioninfo;
	SYSTEM_INFO sysinfo;
	unsigned int proctype;
	char valid_processor_level;
	char *ostype = 0;
	char tmpnodename[MAX_COMPUTERNAME_LENGTH+2];
	size_t tmpnodenamelen = MAX_COMPUTERNAME_LENGTH+1;

	if (!name)
		return -1;

	/* Request simple version info first. */
	versioninfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
	GetVersionEx ((LPOSVERSIONINFO)(&versioninfo));

	valid_processor_level = versioninfo.dwPlatformId == VER_PLATFORM_WIN32_NT 
		|| (versioninfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS && versioninfo.dwMinorVersion >= 10 /*>= win98*/);

	GetSystemInfo (&sysinfo);

	/* CPU type */
	switch (sysinfo.wProcessorArchitecture) {
	case PROCESSOR_ARCHITECTURE_INTEL:
		if (valid_processor_level) {
			if (sysinfo.wProcessorLevel<3)
				proctype = 3;
			else if (sysinfo.wProcessorLevel > 9) /*Pentium 4?*/
				proctype = 6;
			else /*typical*/
				proctype = sysinfo.wProcessorLevel;
		}
		else {
			if (sysinfo.dwProcessorType == PROCESSOR_INTEL_386
				|| sysinfo.dwProcessorType == PROCESSOR_INTEL_486)
				proctype = sysinfo.dwProcessorType / 100;
			else
				proctype = PROCESSOR_INTEL_PENTIUM / 100;
		}
		sprintf (name->machine, "i%d86", proctype);
		break;
	case PROCESSOR_ARCHITECTURE_ALPHA:
		strcpy (name->machine, "alpha");
		break;
	case PROCESSOR_ARCHITECTURE_MIPS:
		strcpy (name->machine, "mips");
		break;
	default:
		strcpy (name->machine, "unknown");
		break;
	}

	strncpy(name->sysname, "Microsoft Windows", 19);

	/* OS Type */
	switch (versioninfo.dwPlatformId) {
	case VER_PLATFORM_WIN32_NT:
		switch (versioninfo.dwMajorVersion) {
		case 3:
		case 4:
			ostype = "NT";
			break;
		case 5:
			switch (versioninfo.dwMinorVersion) {
			case 0:
				ostype = "2000";
				break;
			case 1:
				ostype = "XP";
				break;
			default:
				ostype = "2003";
				break;
			}
		default:
			break;
		}
		break;
	case VER_PLATFORM_WIN32_WINDOWS:
		switch (versioninfo.dwMinorVersion) {
		case 0:
			ostype = "95";
			break;
		case 10:
			ostype = "98";
			break;
		case 90:
			ostype = "Me";
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	if (0==kde_gethostname(tmpnodename, tmpnodenamelen))
		strncpy(name->nodename, tmpnodename, 19);
	else
		name->nodename[0]=0;

	strncpy(name->version, __DATE__, 19); /** @todo ok? */

	if (ostype)
		sprintf(name->release, "%d.%d (%s)", versioninfo.dwMajorVersion, versioninfo.dwMinorVersion, ostype);
	else
		sprintf(name->release, "%d.%d", versioninfo.dwMajorVersion, versioninfo.dwMinorVersion);

	return 0;
}
