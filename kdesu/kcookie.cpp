/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 * 
 * kcookie.cpp: KDE authentication cookies.
 */

#include <stdio.h>
#include <stdlib.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qglobal.h>

#include <kdebug.h>
#include "kcookie.h"

#ifdef __GNUC__
#define ID __PRETTY_FUNCTION__
#else
#define ID "KCookie"
#endif

KCookie::KCookie()
{
    getXCookie();
    getICECookie();
}


QCStringList KCookie::split(QCString line, char ch)
{
    QCStringList result;

    int i=0, pos;
    while ((pos = line.find(ch, i)) != -1) {
	result += line.mid(i, pos-i);
	i = pos+1;
    }
    if (i < (int) line.length())
	result += line.mid(i);
    return result;
}
    

void KCookie::getXCookie()
{    
    char buf[1024];
    FILE *f;

    m_Display = getenv("DISPLAY");
    if (m_Display.isEmpty()) {
	kDebugError("%s: $DISPLAY is not set", ID);
	return;
    }
    QCString cmd;
    cmd.sprintf("xauth list %s", m_Display.data());
    if (!(f = popen(cmd, "r"))) {
	kDebugError("%s: popen(): %m", ID);
	return;
    }
    QCString output = fgets(buf, 1024, f);
    if (pclose(f) < 0) {
	kDebugError("%s: Could not run xauth", ID);
	return;
    }
    output = output.simplifyWhiteSpace();
    QCStringList lst = split(output, ' ');
    if (lst.count() != 3) {
	kDebugError("%s: parse error", ID);
	return;
    }
    m_DisplayAuth = (lst[1] + ' ' + lst[2]);
}


void KCookie::getICECookie()
{
    FILE *f;
    char buf[1024];

    QCString dcopsrv = getenv("DCOPSERVER");
    if (dcopsrv.isEmpty()) {
	QCString home = getenv("HOME");
	if (home.isEmpty()) {
	    kDebugWarning("%s: Cannot find DCOP server.", ID);
	    return;
	}
	if (!(f = fopen(home + "/.DCOPserver", "r"))) {
	    kDebugWarning("%s: Cannot open ~/.DCOPserver.", ID);
	    return;
	}
	dcopsrv = fgets(buf, 1024, f);
	dcopsrv = dcopsrv.stripWhiteSpace();
	fclose(f);
    }
    m_DCOPSrv = split(dcopsrv, ',');
    if (m_DCOPSrv.count() == 0) {
	kDebugWarning("No DCOP servers found");
	return;
    }

    QCStringList::Iterator it;
    for (it=m_DCOPSrv.begin(); it != m_DCOPSrv.end(); it++) {
	QCString cmd;
	cmd.sprintf("iceauth list netid=%s", (*it).data());
	if (!(f = popen(cmd, "r"))) {
	    kDebugError("%s: popen(): %m", ID);
	    break;
	}
	QCStringList output;
	while (fgets(buf, 1024, f) > 0)
	    output += buf;
	if (pclose(f) < 0) {
	    kDebugError("%s: Could not run iceauth.", ID);
	    break;
	}
	QCStringList::Iterator it2;
	for (it2=output.begin(); it2!=output.end(); it2++) {
	    QCStringList lst = split((*it2).simplifyWhiteSpace(), ' ');
	    if (lst.count() != 5) {
		kDebugError("%s: parse error", ID);
		break;
	    }
	    if (lst[0] == "DCOP")
		m_DCOPAuth += (lst[3] + ' ' + lst[4]);
	    else if (lst[0] == "ICE")
		m_ICEAuth += (lst[3] + ' ' + lst[4]);
	    else 
		kDebugError("%s: unknown protocol: %s", ID, lst[0].data());
	}
    }
}

