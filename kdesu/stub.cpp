/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 * 
 * This is free software; you can use this library under the GNU Library 
 * General Public License, version 2. See the file "COPYING.LIB" for the 
 * exact licensing terms.
 *
 * stub.cpp: Conversation with kdesu_stub.
 */

#include <config.h>
#include <stdlib.h>
#include <unistd.h>

#include <qglobal.h>
#include <qcstring.h>
#include <kdatastream.h>

#include <kapplication.h>
#include <kdebug.h>
#include <dcopclient.h>

#include "stub.h"
#include "kcookie.h"


StubProcess::StubProcess()
{
    m_User = "root";
    m_Scheduler = SchedNormal;
    m_Priority = 50;
    m_pCookie = new KCookie;
    m_bXOnly = true;
    m_bDCOPForwarding = false;
}


StubProcess::~StubProcess()
{
    delete m_pCookie;
}


void StubProcess::setPriority(int prio)
{
    if (prio > 100)
	m_Priority = 100;
    else if (prio < 0)
	m_Priority = 0;
    else
	m_Priority = prio;
}


QCString StubProcess::commaSeparatedList(QCStringList lst)
{
    if (lst.count() == 0)
	return QCString("");

    QCStringList::Iterator it = lst.begin();
    QCString str = *it;
    for (it++; it!=lst.end(); it++) 
    {
	str += ',';
	str += *it;
    }
    return str;
}
    
/*
 * Conversation with kdesu_stub. This is how we pass the authentication
 * tokens (X11, DCOP) and other stuff to kdesu_stub.
 * return values: -1 = error, 0 = ok, 1 = kill me
 */

int StubProcess::ConverseStub(int check)
{
    QCString line, tmp;
    while (1) 
    {
	line = readLine();
	if (line.isNull())
	    return -1;

	if (line == "kdesu_stub") 
	{
	    // This makes parsing a lot easier.
	    enableLocalEcho(false);
	    if (check) writeLine("stop");
	    else writeLine("ok");
	} else if (line == "display") {
	    writeLine(display());
	} else if (line == "display_auth") {
#ifdef Q_WS_X11
	    writeLine(displayAuth());
#else
	    writeLine("");
#endif
	} else if (line == "dcopserver") {
	    if (m_bDCOPForwarding)
	       writeLine(dcopServer());
	    else
	       writeLine("no");
	} else if (line == "dcop_auth") {
	    if (m_bDCOPForwarding)
	       writeLine(dcopAuth());
	    else
	       writeLine("no");
	} else if (line == "ice_auth") {
	    if (m_bDCOPForwarding)
	       writeLine(iceAuth());
	    else
	       writeLine("no");
	} else if (line == "command") {
	    writeLine(m_Command);
	} else if (line == "path") {
	    QCString path = getenv("PATH");
	    if (m_User == "root") 
		if (!path.isEmpty())
		    path = "/sbin:/usr/sbin:" + path;
		else
		    path = "/sbin:/usr/sbin";
	    writeLine(path);
	} else if (line == "user") {
	    writeLine(m_User);
	} else if (line == "priority") {
	    tmp.setNum(m_Priority);
	    writeLine(tmp);
	} else if (line == "scheduler") {
	    if (m_Scheduler == SchedRealtime) writeLine("realtime");
	    else writeLine("normal");
	} else if (line == "xwindows_only") {
	    if (m_bXOnly) writeLine("no");
	    else writeLine("yes");
	} else if (line == "app_start_pid") {
	    tmp.setNum(getpid());
	    writeLine(tmp);
	} else if (line == "end") {
	    return 0;
	} else 
	{
	    kdWarning(900) << k_lineinfo << "Unknown request: -->" << line 
		           << "<--\n";
	    return 1;
	}
    }

    return 0;
}


void StubProcess::notifyTaskbar(const QString &suffix)
{
    QString exec, icon;
    int sp = m_Command.find(" ");
    if (sp != -1) 
	exec = m_Command.left(sp);
    else
	exec = m_Command;
    icon = exec;
    if (!suffix.isEmpty())
    {
	exec += " ";
	exec += suffix;
    }

    QByteArray params;
    QDataStream stream(params, IO_WriteOnly);
    stream << exec << icon << getpid();
    DCOPClient *client = kapp->dcopClient();
    if (!client->isAttached())
	client->attach();
    client->send( "kicker", "TaskbarApplet",
	          "clientStarted(QString,QString,pid_t)", params );
}

void StubProcess::virtual_hook( int id, void* data )
{ PtyProcess::virtual_hook( id, data ); }
