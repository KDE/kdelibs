/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 * 
 * stub.cpp: Conversation with kdesu_stub.
 */

#include <config.h>
#include <stdlib.h>

#include <qglobal.h>
#include <qcstring.h>

#include <kdebug.h>

#include "stub.h"
#include "kcookie.h"

#ifdef __GNUC__
#define ID __PRETTY_FUNCTION__
#else
#define ID "StubProcess"
#endif


StubProcess::StubProcess()
{
    m_User = "root";
    m_Scheduler = SchedNormal;
    m_Priority = 50;
    m_pCookie = new KCookie;
    m_bXOnly = false;
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
    for (it++; it!=lst.end(); it++) {
	str += ',';
	str += *it;
    }
    return str;
}
    
/*
 * Conversation with kdesu_stub. This is how we pass the authentication
 * tokens (X11, DCOP) and other stuff to kdesu_stub.
 */

int StubProcess::ConverseStub(bool check_only)
{
    // This makes parsing a lot easier.
    enableLocalEcho(false);

    QCString line, tmp;
    while (1) {
	line = readLine();
	if (line.isNull())
	    return -1;
	if (line == "kdesu_stub") {
	    if (check_only) writeLine("stop");
	    else writeLine("ok");
	} else if (line == "display") {
	    writeLine(display());
	} else if (line == "display_auth") {
	    writeLine(displayAuth());
	} else if (line == "dcopserver") {
	    writeLine(commaSeparatedList(dcopServer()));
	} else if (line == "dcop_auth") {
	    writeLine(commaSeparatedList(dcopAuth()));
	} else if (line == "ice_auth") {
	    writeLine(commaSeparatedList(iceAuth()));
	} else if (line == "command") {
	    writeLine(m_Command);
	} else if (line == "path") {
	    writeLine(getenv("PATH"));
	} else if (line == "user") {
	    writeLine(m_User);
	} else if (line == "priority") {
	    writeLine(tmp.setNum(m_Priority));
	} else if (line == "scheduler") {
	    if (m_Scheduler == SchedRealtime) writeLine("realtime");
	    else writeLine("normal");
	} else if (line == "build_sycoca") {
	    if (m_bXOnly) writeLine("no");
	    else writeLine("yes");
	} else if (line == "end") {
	    return 0;
	} else {
	    kDebugWarning("%s: Unknown request: -->%s<--", ID, line.data());
	    return -1;
	}
    }
    return 0;
}

