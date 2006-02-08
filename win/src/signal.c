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

#include <winposix_export.h>
#include <windows.h>

#include <sys/types.h>
#include <errno.h>

KDEWIN32_EXPORT int kill(pid_t pid, int sig)
{
	int	ret;
	HANDLE h = OpenProcess(PROCESS_TERMINATE,FALSE,(DWORD)pid);
	ret = (h != NULL)?0:ESRCH;
	if(h) 
	{
		TerminateProcess(h,sig);
		CloseHandle(h);
	}
	return ret;
}

KDEWIN32_EXPORT pid_t waitpid(pid_t p, int *a, int b)
{
	int	ret;
	HANDLE h = OpenProcess(PROCESS_TERMINATE,FALSE,(DWORD)p);
	ret = (h != NULL)?p:-1;
	if(h) 
	{
		DWORD dw;
		WaitForSingleObject(h,INFINITE);
		GetExitCodeProcess(h,&dw);
		CloseHandle(h);
	}
	else
		errno = ECHILD;
	return ret;
}

