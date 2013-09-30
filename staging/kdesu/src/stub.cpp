/* vi: ts=8 sts=4 sw=4
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

#include "stub.h"

#include <config-kdesu.h>

#include "kcookie.h"

#include <stdlib.h>
#include <unistd.h>



#include <QDebug>

extern int kdesuDebugArea();

namespace KDESu {

using namespace KDESuPrivate;

StubProcess::StubProcess()
    : d(0)
{
    m_User = "root";
    m_Scheduler = SchedNormal;
    m_Priority = 50;
    m_pCookie = new KCookie;
    m_bXOnly = true;
}


StubProcess::~StubProcess()
{
    delete m_pCookie;
}


void StubProcess::setCommand(const QByteArray &command)
{
    m_Command = command;
}


void StubProcess::setUser(const QByteArray &user)
{
    m_User = user;
}


void StubProcess::setXOnly(bool xonly)
{
    m_bXOnly = xonly;
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


void StubProcess::setScheduler(int sched)
{
    m_Scheduler = sched;
}


QByteArray StubProcess::commaSeparatedList(const QList<QByteArray> &lst)
{
    QByteArray str;
    for (int i = 0; i < lst.count(); ++i) {
        str += ',';
        str += lst.at(i);
    }
    return str;
}

void StubProcess::writeString(const QByteArray &str)
{
    QByteArray out;
    out.reserve(str.size() + 8);
    for (int i = 0; i < str.size(); i++) {
        uchar c = str.at(i);
        if (c < 32) {
            out.append('\\');
            out.append(c + '@');
        } else if (c == '\\') {
            out.append('\\');
            out.append('/');
        } else {
            out.append(c);
        }
    }
    writeLine(out);
}

/*
 * Map pid_t to a signed integer type that makes sense for QByteArray;
 * only the most common sizes 16 bit and 32 bit are special-cased.
 */
template<int T> struct PIDType { typedef pid_t PID_t; } ;
template<> struct PIDType<2> { typedef qint16 PID_t; } ;
template<> struct PIDType<4> { typedef qint32 PID_t; } ;

/*
 * Conversation with kdesu_stub. This is how we pass the authentication
 * tokens (X11) and other stuff to kdesu_stub.
 * return values: -1 = error, 0 = ok, 1 = kill me
 */

int StubProcess::ConverseStub(int check)
{
    QByteArray line, tmp;

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
	    break;
	} 
    }

    while (1)
    {
	line = readLine();
	if (line.isNull())
	    return -1;

        if (line == "display") {
	    writeLine(display());
	} else if (line == "display_auth") {
#if HAVE_X11
	    writeLine(displayAuth());
#else
	    writeLine("");
#endif
	} else if (line == "command") {
	    writeString(m_Command);
	} else if (line == "path") {
	    QByteArray path = qgetenv("PATH");
            if (!path.isEmpty() && path[0] == ':')
                path = path.mid(1);
	    if (m_User == "root") {
		if (!path.isEmpty())
		    path = "/sbin:/bin:/usr/sbin:/usr/bin:" + path;
		else
		    path = "/sbin:/bin:/usr/sbin:/usr/bin";
            }
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
	} else if (line == "app_startup_id") {
	    QList<QByteArray> env = environment();
	    QByteArray tmp;
	    for(int i = 0; i < env.count(); ++i)
	    {
                const char startup_env[] = "DESKTOP_STARTUP_ID=";
                if (env.at(i).startsWith(startup_env))
                    tmp = env.at(i).mid(sizeof(startup_env) - 1);
	    }
	    if( tmp.isEmpty())
		tmp = "0"; // krazy:exclude=doublequote_chars
	    writeLine(tmp);
	} else if (line == "app_start_pid") { // obsolete
	    // Force the pid_t returned from getpid() into
	    // something QByteArray understands; avoids ambiguity
	    // between short and unsigned short in particular.
	    tmp.setNum((PIDType<sizeof(pid_t)>::PID_t)(getpid()));
	    writeLine(tmp);
	} else if (line == "environment") { // additional env vars
	    QList<QByteArray> env = environment();
            for (int i = 0; i < env.count(); ++i)
                writeString(env.at(i));
	    writeLine( "" );
	} else if (line == "end") {
	    return 0;
	} else
	{
	    qWarning() << "[" << __FILE__ << ":" << __LINE__ << "] " << "Unknown request:" << line;
	    return 1;
	}
    }

    return 0;
}


QByteArray StubProcess::display()
{
    return m_pCookie->display();
}


QByteArray StubProcess::displayAuth()
{
#if HAVE_X11
    return m_pCookie->displayAuth();
#else
    return QByteArray();
#endif
}


void StubProcess::virtual_hook( int id, void* data )
{ PtyProcess::virtual_hook( id, data ); }

}
