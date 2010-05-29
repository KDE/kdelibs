/* vi: ts=8 sts=4 sw=4
 *
 * $Id: kcookie.h 427785 2005-06-21 20:25:37Z adridg $
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

class KDESU_EXPORT KCookie
{
public:
    KCookie();

    /**
     * Returns the X11 display.
     */
    QCString display() { return m_Display; }

#ifdef Q_WS_X11
    /**
     * Returns the X11 magic cookie, if available.
     */
    QCString displayAuth() { return m_DisplayAuth; }
#endif

    /**
     * Select the DCOP transport to look for. Default: "local"
     */
    void setDcopTransport(const QCString &dcopTransport);

    /**
     * Returns the netid where the dcopserver is running
     */
    QCString dcopServer();

    /** 
     * Returns a list of magic cookies for DCOP protocol authentication. 
     * The order is the same as in dcopServer().
     */
    QCString dcopAuth();

    /**
     * Returns a list of magic cookies for the ICE protocol.
     */
    QCString iceAuth();

private:
    void getXCookie();
    void getICECookie();
    QCStringList split(const QCString &line, char ch);

    void blockSigChild();
    void unblockSigChild();

    bool m_bHaveDCOPCookies;
    bool m_bHaveICECookies;

    QCString m_Display;
#ifdef Q_WS_X11
    QCString m_DisplayAuth;
#endif
    QCString m_DCOPSrv;
    QCString m_DCOPAuth;
    QCString m_ICEAuth;
    QCString m_dcopTransport;

    class KCookiePrivate;
    KCookiePrivate *d;
};


#endif // __KCookie_h_Included__
