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
 */

#ifndef __Stub_h_Included__
#define __Stub_h_Included__

#include <qcstring.h>
#include <qvaluelist.h>

#include "process.h"
#include "kcookie.h"

typedef QValueList<QCString> QCStringList;

/**
 * Chat with kdesu_stub.
 *
 * StubProcess extends PtyProcess with functionality to chat with kdesu_stub.
 */

class StubProcess: public PtyProcess
{
public:
    StubProcess();
    ~StubProcess();

    /** Set the command. */
    void setCommand(QCString command) { m_Command = command; }

    /** Set the target user.  */
    void setUser(QCString user) { m_User = user; }

    /**
     * Set to "X only mode": DCOP is not forwarded, the sycoca is not
     * built and kdeinit is not launched.
     */
    void setXOnly(bool xonly) { m_bXOnly = xonly; }

    /**
     * Set the priority of the process. The priority value must be between 0
     * and 100, 0 being the lowest priority. This value is mapped to the
     * scheduler and system dependant priority range of the OS.
     */
    void setPriority(int prio);

    /**
     * Different schedulers. SchedNormal is the normal Unix timesharing
     * scheduler, while SchedRealtime is a POSIX.1b realtime scheduler.
     */
    enum Scheduler { SchedNormal, SchedRealtime };

    /**
     * Set the scheduler type. 
     */
    void setScheduler(int sched) { m_Scheduler = sched; }

protected:

    /** Exchange all parameters with kdesu_stub. */
    int ConverseStub(int check);

    /** Notify the taskbar that a new application has been started. */
    void notifyTaskbar(QString suffix);

    /** 
     * This virtual function can be overloaded when special behaviour is
     * desired. By default, it returns the value returned by @ref #KCookie.
     */
    virtual QCString display() { return m_pCookie->display(); }
#ifdef _WS_X11_
    /** See @ref #display. */
    virtual QCString displayAuth() { return m_pCookie->displayAuth(); }
#endif
    /** See @ref #display. */
    virtual QCStringList dcopServer() { return m_pCookie->dcopServer(); }
    /** See @ref #display. */
    virtual QCStringList dcopAuth() { return m_pCookie->dcopAuth(); }
    /** See @ref #display. */
    virtual QCStringList iceAuth() { return m_pCookie->iceAuth(); }

    bool m_bXOnly;
    int m_Priority, m_Scheduler;
    QCString m_Command, m_User;
    KCookie *m_pCookie;
    
private:
    QCString commaSeparatedList(QCStringList);

    class StubProcessPrivate;
    StubProcessPrivate *d;
};

#endif // __Stub_h_Included__
