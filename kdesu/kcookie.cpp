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
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qglobal.h>
#include <qfile.h>

#include <dcopclient.h>

#include <kdebug.h>
#include <kprocess.h>
#include "kcookie.h"


KCookie::KCookie()
{
#ifdef Q_WS_X11
    getXCookie();
#endif
    setDcopTransport("local");
}

void KCookie::setDcopTransport(const QCString &dcopTransport)
{
    m_dcopTransport = dcopTransport;
    m_bHaveDCOPCookies = false;
    m_bHaveICECookies = false;
    m_DCOPSrv = "";
    m_DCOPAuth = "";
    m_ICEAuth = "";
}    

QCStringList KCookie::split(const QCString &line, char ch)
{
    QCStringList result;

    int i=0, pos;
    while ((pos = line.find(ch, i)) != -1) 
    {
	result += line.mid(i, pos-i);
	i = pos+1;
    }
    if (i < (int) line.length())
	result += line.mid(i);
    return result;
}
    
void KCookie::blockSigChild()
{
    sigset_t sset;
    sigemptyset(&sset);
    sigaddset(&sset, SIGCHLD);
    sigprocmask(SIG_BLOCK, &sset, 0L);
}

void KCookie::unblockSigChild()
{
    sigset_t sset;
    sigemptyset(&sset);
    sigaddset(&sset, SIGCHLD);
    sigprocmask(SIG_UNBLOCK, &sset, 0L);
}

void KCookie::getXCookie()
{
    char buf[1024];
    FILE *f;

#ifdef Q_WS_X11
    m_Display = getenv("DISPLAY");
#else
    m_Display = getenv("QWS_DISPLAY");
#endif
    if (m_Display.isEmpty()) 
    {
	kdError(900) << k_lineinfo << "$DISPLAY is not set.\n";
	return;
    }
#ifdef Q_WS_X11 // No need to mess with X Auth stuff
    QCString disp = m_Display;
    if (disp.left(10) == "localhost:")
       disp = disp.mid(9);

    QString cmd = "xauth list "+KProcess::quote(disp);
    blockSigChild(); // pclose uses waitpid()
    if (!(f = popen(QFile::encodeName(cmd), "r"))) 
    {
	kdError(900) << k_lineinfo << "popen(): " << perror << "\n";
	unblockSigChild();
	return;
    }
    QCString output = fgets(buf, 1024, f);
    if (pclose(f) < 0) 
    {
	kdError(900) << k_lineinfo << "Could not run xauth.\n";
	unblockSigChild();
	return;
    }
    unblockSigChild();
    output = output.simplifyWhiteSpace();
    if (output.isEmpty())
    {
       kdWarning(900) << "No X authentication info set for display " <<
       m_Display << endl; return;
    }
    QCStringList lst = split(output, ' ');
    if (lst.count() != 3) 
    {
	kdError(900) << k_lineinfo << "parse error.\n";
	return;
    }
    m_DisplayAuth = (lst[1] + ' ' + lst[2]);
#endif
}

void KCookie::getICECookie()
{
    FILE *f;
    char buf[1024];

    QCString dcopsrv = getenv("DCOPSERVER");
    if (dcopsrv.isEmpty()) 
    {
	QCString dcopFile = DCOPClient::dcopServerFile();
	if (!(f = fopen(dcopFile, "r"))) 
	{
	    kdWarning(900) << k_lineinfo 
					   << "Cannot open " << dcopFile << ".\n";
	    return;
	}
	dcopsrv = fgets(buf, 1024, f);
	dcopsrv = dcopsrv.stripWhiteSpace();
	fclose(f);
    }
    QCStringList dcopServerList = split(dcopsrv, ',');
    if (dcopServerList.count() == 0) 
    {
	kdError(900) << k_lineinfo << "No DCOP servers found.\n";
	return;
    }

    QCStringList::Iterator it;
    for (it=dcopServerList.begin(); it != dcopServerList.end(); it++) 
    {
        if (strncmp((*it).data(), m_dcopTransport.data(), m_dcopTransport.length()) != 0)
            continue;
        m_DCOPSrv = *it;
	QString cmd = "iceauth list "+KProcess::quote("netid="+m_DCOPSrv);
	blockSigChild();
	if (!(f = popen(QFile::encodeName(cmd), "r"))) 
	{
	    kdError(900) << k_lineinfo << "popen(): " << perror << "\n";
	    unblockSigChild();
	    break;
	}
	QCStringList output;
	while (fgets(buf, 1024, f) > 0)
	    output += buf;
	if (pclose(f) < 0) 
	{
	    kdError(900) << k_lineinfo << "Could not run iceauth.\n";
	    unblockSigChild();
	    break;
	}
	unblockSigChild();
	QCStringList::Iterator it2;
	for (it2=output.begin(); it2!=output.end(); it2++) 
	{
	    QCStringList lst = split((*it2).simplifyWhiteSpace(), ' ');
	    if (lst.count() != 5) 
	    {
		kdError(900) << "parse error.\n";
		break;
	    }
	    if (lst[0] == "DCOP")
		m_DCOPAuth = (lst[3] + ' ' + lst[4]);
	    else if (lst[0] == "ICE")
		m_ICEAuth = (lst[3] + ' ' + lst[4]);
	    else 
		kdError(900) << k_lineinfo << "unknown protocol: " << lst[0] << "\n";
	}
	break;
    }
    m_bHaveDCOPCookies = true;
    m_bHaveICECookies = true;
}

QCString KCookie::dcopServer() 
{ 
   if (!m_bHaveDCOPCookies)
      getICECookie();
   return m_DCOPSrv; 
}

QCString KCookie::dcopAuth() 
{ 
   if (!m_bHaveDCOPCookies)
      getICECookie();
   return m_DCOPAuth; 
}

QCString KCookie::iceAuth() 
{ 
   if (!m_bHaveICECookies)
      getICECookie(); 
   return m_ICEAuth; 
}
