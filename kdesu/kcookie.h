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

#include <q3cstring.h>
#include <q3valuelist.h>

typedef Q3ValueList<Q3CString> QCStringList;


/**
 * Utility class to access the authentication tokens needed to run a KDE
 * program (X11 and DCOP cookies).
 */

class KCookie
{
public:
    KCookie();

    /**
     * Returns the X11 display.
     */
    Q3CString display() { return m_Display; }

#ifdef Q_WS_X11
    /**
     * Returns the X11 magic cookie, if available.
     */
    Q3CString displayAuth() { return m_DisplayAuth; }
#endif

    /**
     * Select the DCOP transport to look for. Default: "local"
     */
    void setDcopTransport(const Q3CString &dcopTransport);

    /**
     * Returns the netid where the dcopserver is running
     */
    Q3CString dcopServer();

    /** 
     * Returns a list of magic cookies for DCOP protocol authentication. 
     * The order is the same as in dcopServer().
     */
    Q3CString dcopAuth();

    /**
     * Returns a list of magic cookies for the ICE protocol.
     */
    Q3CString iceAuth();

private:
    void getXCookie();
    void getICECookie();
    QCStringList split(const Q3CString &line, char ch);

    void blockSigChild();
    void unblockSigChild();

    bool m_bHaveDCOPCookies;
    bool m_bHaveICECookies;

    Q3CString m_Display;
#ifdef Q_WS_X11
    Q3CString m_DisplayAuth;
#endif
    Q3CString m_DCOPSrv;
    Q3CString m_DCOPAuth;
    Q3CString m_ICEAuth;
    Q3CString m_dcopTransport;

    class KCookiePrivate;
    KCookiePrivate *d;
};


#endif // __KCookie_h_Included__
