/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 * 
 */

#ifndef __Stub_h_Included__
#define __Stub_h_Included__

#include <qcstring.h>
#include <qvaluelist.h>

#include "process.h"
#include "kcookie.h"

typedef QValueList<QCString> QCStringList;

/**
 * StubProcess extends PtyProcess with functionality to chat 
 * with kdesu_stub.
 */

class StubProcess: public PtyProcess
{
public:
    StubProcess();
    ~StubProcess();

    /**
     * Set the command.
     */
    void setCommand(QCString command) { m_Command = command; }

    /**
     * Set the target user.
     */
    void setUser(QCString user) { m_User = user; }

    /**
     * Set to "X only mode": DCOP is not forwarded and the sycoca is not
     * built. Relevant only when executing kdesu_stub.
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
    /**
     * Have a conversation with kdesu_stub.
     */
    int ConverseStub(bool check_only);

    /** 
     * These virtual functions can be overloaded when special behaviour is
     * desired.
     */
    virtual QCString display() { return m_pCookie->display(); }
    virtual QCString displayAuth() { return m_pCookie->displayAuth(); }
    virtual QCStringList dcopServer() { return m_pCookie->dcopServer(); }
    virtual QCStringList dcopAuth() { return m_pCookie->dcopAuth(); }
    virtual QCStringList iceAuth() { return m_pCookie->iceAuth(); }

    bool m_bXOnly;
    int m_Priority, m_Scheduler;
    QCString m_Command, m_User;
    KCookie *m_pCookie;
    
private:
    QCString commaSeparatedList(QCStringList);
};

#endif // __Stub_h_Included__
