/*
   This file is part of the KDE libraries
   Copyright (C) 2005 Andreas Roth <aroth@arsoft-online.com>

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

//this file is included by dcopserver.cpp

#include <qeventloop.h>

DWORD WINAPI DCOPServer::TerminatorThread(void * pParam)
{
    DCOPServer * server = (DCOPServer*)pParam;

    WaitForSingleObject(server->m_evTerminate,INFINITE);
	fprintf( stderr, "DCOPServer : terminate event signaled\n" );
    if(!server->shutdown) {
		
		ResetEvent(server->m_evTerminate);
        server->slotShutdown();

		// Wait for main thread to tell us to realy terminate now
		// Need some further event processing to get the timer signals
		while(WaitForSingleObject(server->m_evTerminate,100) != WAIT_OBJECT_0)
			QApplication::eventLoop()->processEvents(QEventLoop::ExcludeUserInput|QEventLoop::ExcludeSocketNotifiers);
		fprintf( stderr, "DCOPServer : terminated event signaled the last time\n" );
    }
	fprintf( stderr, "DCOPServer : terminate thread teminated\n" );
    return 0;
}

BOOL WINAPI DCOPServer::dcopServerConsoleProc(DWORD dwCtrlType)
{
    BOOL ret;
    switch(dwCtrlType)
    {
	case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
    case CTRL_C_EVENT:
        system(findDcopserverShutdown()+" --nokill");
        ret = true;
        break;
    default:
        ret = false;
    }
    return ret;
}

