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
	kdError() << ID << ": $DISPLAY is not set\n";
	return;
    }
    QCString cmd;
    cmd.sprintf("xauth list %s", m_Display.data());
    if (!(f = popen(cmd, "r"))) {
        kdError() << ID << ": popen(): " << perror << endl;
        return;
    }
    QCString output = fgets(buf, 1024, f);
    if (pclose(f) < 0) {
        kdError() << "ID" << ": Could not run xauth\n";
        return;
    }
    output = output.simplifyWhiteSpace();
    QCStringList lst = split(output, ' ');
    if (lst.count() != 3) {
        kdError() << ID << ": parse error\n";
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
	    kdWarning() << ID << ": Cannot find DCOP server.\n";
	    return;
	}
	if (!(f = fopen(home + "/.DCOPserver", "r"))) {
	    kdWarning() << ID << ": Cannot open ~/.DCOPserver.\n";
	    return;
	}
	dcopsrv = fgets(buf, 1024, f);
	dcopsrv = dcopsrv.stripWhiteSpace();
	fclose(f);
    }
    m_DCOPSrv = split(dcopsrv, ',');
    if (m_DCOPSrv.count() == 0) {
	kdWarning() << "No DCOP servers found\n";
	return;
    }

    QCStringList::Iterator it;
    for (it=m_DCOPSrv.begin(); it != m_DCOPSrv.end(); it++) {
	QCString cmd;
	cmd.sprintf("iceauth list netid=%s", (*it).data());
	if (!(f = popen(cmd, "r"))) {
	    kdError() << ID << ": popen(): " << perror << endl;
	    break;
	}
	QCStringList output;
	while (fgets(buf, 1024, f) > 0)
	    output += buf;
	if (pclose(f) < 0) {
	    kdError() << ID << ": Could not run iceauth.\n";
	    break;
	}
	QCStringList::Iterator it2;
	for (it2=output.begin(); it2!=output.end(); it2++) {
	    QCStringList lst = split((*it2).simplifyWhiteSpace(), ' ');
	    if (lst.count() != 5) {
		kdError() << ID << ": parse error\n";
		break;
	    }
	    if (lst[0] == "DCOP")
		m_DCOPAuth += (lst[3] + ' ' + lst[4]);
	    else if (lst[0] == "ICE")
		m_ICEAuth += (lst[3] + ' ' + lst[4]);
	    else
		kdError() << ID << ": unknown protocol: " << lst[0] << endl;
	}
    }
}

