/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * This is free software; you can use this library under the GNU Library 
 * General Public License, version 2. See the file "COPYING.LIB" for the 
 * exact licensing terms.
 */

#ifndef __KCookie_h_Included__
#define __KCookie_h_Included__

#include <qcstring.h>
#include <qvaluelist.h>

typedef QValueList<QCString> QCStringList;


/**
 * Utility class to access the authentication tokens needed to run a KDE
 * program (X11 and DCOP cookies).
 */

class KCookie
{
public:
    KCookie();

    /** Returns the X11 display. */
    QCString display() { return m_Display; }

#ifdef Q_WS_X11
    /** Returns the X11 magic cookie, if available. */
    QCString displayAuth() { return m_DisplayAuth; }
#endif

    /** Returns a list of netid's where a dcopserver is running */
    QCStringList dcopServer() { return m_DCOPSrv; }

    /** 
     * Returns a list of magic cookies for DCOP protocol authentication. 
     * The order is the same as in dcopServer().
     */
    QCStringList dcopAuth() { return m_DCOPAuth; }

    /** Returns a list of magic cookies for the ICE protocol. */
    QCStringList iceAuth() { return m_ICEAuth; }

private:
    void getXCookie();
    void getICECookie();
    QCStringList split(QCString line, char ch);

    void blockSigChild();
    void unblockSigChild();

    QCString m_Display;
#ifdef Q_WS_X11
    QCString m_DisplayAuth;
#endif
    QCStringList m_DCOPSrv, m_DCOPAuth, m_ICEAuth;

    class KCookiePrivate;
    KCookiePrivate *d;
};


#endif // __KCookie_h_Included__
