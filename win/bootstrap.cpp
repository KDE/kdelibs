/*
   This file is part of the KDE libraries
   Copyright (C) 2004 Jaroslaw Staniek <js@iidea.pl>

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
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <sys/file.h>
#include <stdio.h>

#include "qeventloopex.h"

QEventLoopEx* qeventloopex = 0;

/**
 Actions to perform at very early stage of KDE application life on MS Windows.
 Currently not much is performed here but later, who knows...

 Additional algorithm for win9x (including Millenium), where are problems with 
 easy setting environment variables:

 - try to find HOME env. variable
 - if not found, try to find USERPROFILE env. variable
 - if not found, try to find both HOMEDRIVE and HOMEPATH env. variables
 - if not found, try in the Windows Registry:
   - try get 'Software\KDE' value from HKEY_CURRENT_USER section of Windows Registry
   - if not found, try to get from 
     'Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders\AppData'
   - if one of above two found, put the value as HOME environment variable 
     using putenv() function.

 Once HOME variable is set, Qt handles it well.
*/
KDEWIN32_EXPORT void kde_bootstrap()
{
	OSVERSIONINFOA osver;
	osver.dwOSVersionInfoSize = sizeof(osver);
	DWORD rc = GetVersionExA( &osver );

	WSADATA wsadata;
	WSAStartup(MAKEWORD(2,2),&wsadata);

	qeventloopex = new QEventLoopEx();

	//for win9x only:
	if (osver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
		//set $HOME if not available
		char *s, *s2;
		s = getenv("HOME");
		if (!s)
			s = getenv("USERPROFILE");
		if (!s) {
			s = getenv("HOMEDRIVE");
			s2 = getenv("HOMEPATH");
			if (!s2)
				s = 0;
		}
		if (!s) {
			//no $HOME! : set a value from registry:
			HKEY hKey;
			DWORD  len;
			char path[1024];
			char path2[1024];
			bool ok;
#define KEY "Software\\KDE"
			ok = (ERROR_SUCCESS == RegOpenKeyExA( HKEY_CURRENT_USER, KEY, 0, KEY_QUERY_VALUE, &hKey ));
			if (ok) {
				len = sizeof(path);
				ok = (ERROR_SUCCESS == RegQueryValueExA( hKey, "HOME", 0, 0, (LPBYTE)path, &len ));
				fprintf(stderr,"RegQueryValueExA = %d, %s\n", ok, path);
			}
#define KEY2 "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"
			if (!ok) {
				ok = (ERROR_SUCCESS == RegOpenKeyExA( HKEY_CURRENT_USER, KEY, 0, KEY_QUERY_VALUE, &hKey ));
				if (ok) {
					len = sizeof(path);
					ok = (ERROR_SUCCESS == RegQueryValueExA( hKey, "AppData", 0, 0, (LPBYTE)path, &len ));
					fprintf(stderr,"RegQueryValueExA = %d, %s\n", ok, path);
				}
			}
			if (ok) {
				ok = (0==access(path, R_OK));
				if (!ok) {
					CreateDirectoryA(path,NULL);
					ok = (0==access(path, R_OK));
					fprintf(stderr,"CreateDirectoryA(%s) = %d\n", path, ok);
				}

				if (ok) {
					//it's not a problem with encoding, because Qt will use fromLocal8Bit()
					strcpy(path2, "HOME=");
					strncat(path2, path, sizeof(path2)-1-strlen(path2));
					rc = putenv(path2);
					fprintf(stderr,"putenv(HOME) = %d\n",(int)rc);
/*
					path[0]=0;
					char *p = getenv( "HOME" );
					fprintf(stderr,"getenv(HOME) = %s\n", p);*/
				}
				else
					fprintf(stderr,"'%s' doesn't exist\n",path);
			}
			else
				fprintf(stderr,"$HOME not found!\n",path);

			RegCloseKey( hKey );
		}
	}
}

/**
 Actions to perform after destroying KDE application on MS Windows.

 Currently, custom even loop (QEventLoopEx) is deleted here.
*/
KDEWIN32_EXPORT void kde_destroy()
{
	delete qeventloopex;
}
