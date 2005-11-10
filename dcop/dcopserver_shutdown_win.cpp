/*
  This file is part of the KDE libraries
  Copyright (c) 1999 Waldo Bastian <bastian@kde.org>
            (c) 1999 Mario Weilguni <mweilguni@sime.com>
            (c) 2001 Lubos Lunak <l.lunak@kde.org>

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <sys/socket.h>
#include <stdlib.h>
#if 0
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/un.h>

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <signal.h>
#endif

#define QT_CLEAN_NAMESPACE 1
#include <qfile.h>

#include <dcopclient.h>

#define BUFFER_SIZE 4096

extern QCString dcopServerFile(const QCString &hostname, bool old);

static char *getDisplay()
{
   const char *display;
   char *result;
   char *screen;
   char *colon;
/*
 don't test for a value from qglobal.h but instead distinguish
 Qt/X11 from Qt/Embedded by the fact that Qt/E apps have -DQWS
 on the commandline (which in qglobal.h however triggers Q_WS_QWS,
 but we don't want to include that here) (Simon)
#ifdef Q_WS_X11
 */
#if !defined(QWS)
   display = getenv("DISPLAY");
#else
   display = getenv("QWS_DISPLAY");
#endif
   if (!display || !*display)
   {
      display = "NODISPLAY";
   }
   result = (char*)malloc(strlen(display)+1);
   if (result == NULL)
      return NULL;
   strcpy(result, display);
   screen = strrchr(result, '.');
   colon = strrchr(result, ':');
   if (screen && (screen > colon))
      *screen = '\0';
   return result;
}

static void cleanupDCOPsocket(const char *socketfile)
{
   char cmd[BUFFER_SIZE];
   char buffer[BUFFER_SIZE];
   const char *socket_file;
   int l; 

   l = strlen(socketfile);
   if (!l)
      return;
   strncpy(buffer,socketfile,l);
   buffer[l-1] = '\0'; /* strip LF */

   socket_file = strchr(buffer, ':');
   if (socket_file)
     socket_file++;

   if (socket_file)
      unlink(socket_file);

   snprintf(cmd, BUFFER_SIZE, "iceauth remove netid='%s'", buffer);
   system(cmd);
}

#ifdef Q_OS_WIN
static void killDCOPWin(pid_t pid)
{
	char	sz[256];
	sprintf(sz,"dcopserver%i",pid);
	HANDLE hEvent = CreateEventA(NULL,true,false,(LPCSTR)sz);
	DWORD dwError = GetLastError();
	printf("Signal event %s %p, %i\n",sz,hEvent,dwError);
	if(hEvent != NULL)
	{
		SetEvent(hEvent);
		CloseHandle(hEvent);
	}
}
#endif

static void cleanupDCOP(int dont_kill_dcop, int wait_for_exit)
{
	QCString host;
	QCString strDCOPServer = DCOPClient::dcopServerFile(host);

	if(strDCOPServer.isEmpty())
	{
			printf("no server file\n");
		return;
	}
	printf("server file %s\n",(const char *)strDCOPServer);

	pid_t	pid = 0;
	QFile f(strDCOPServer);
	if(f.open(QIODevice::ReadOnly))
	{
		QString str;
		while(f.readLine(str,2048))
		{
			pid = str.toULong();
			if (pid)
				break;
			cleanupDCOPsocket(str.ascii());
		}
	}
	f.close();
	/* Clean up .DCOPserver file */
	QFile::remove(strDCOPServer);
	printf("remove server file %s\n",(const char *)strDCOPServer);

	if(pid)
	{
		if(!dont_kill_dcop)
		{
#ifdef Q_OS_WIN
			killDCOPWin(pid);
#else
			kill(pid, SIGTERM);
#endif
		}
		else
		{
#ifdef Q_OS_WIN
			killDCOPWin(pid);
#endif
		}
	}

#ifdef Q_OS_WIN
	if(wait_for_exit)
	{
		HANDLE hProcess = OpenProcess(SYNCHRONIZE,false,(DWORD)pid);
		if(hProcess)
		{
			WaitForSingleObject(hProcess,INFINITE);
			CloseHandle(hProcess);
		}
	}
#else
	while(wait_for_exit && (kill(pid, 0) == 0))
	{
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 100000;
		select(0,0,0,0,&tv);
	}
#endif
}

int main(int argc, char **argv)
{
	QCString host;
	
   int dont_kill_dcop = (argc == 2) && (strcmp(argv[1], "--nokill") == 0);
   int wait_for_exit = (argc == 2) && (strcmp(argv[1], "--wait") == 0);

   cleanupDCOP(dont_kill_dcop, wait_for_exit);
   return 0;
}
