/* vi: ts=8 sts=4 sw=4
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

#include <QtCore/QByteRef>
#include <QtCore/QList>

#include "process.h"

#include <kdesu/kdesu_export.h>

namespace KDESu {
    
namespace KDESuPrivate { class KCookie; }


/** \class StubProcess stub.h kdesu/stub.h
 * Chat with kdesu_stub.
 *
 * StubProcess extends PtyProcess with functionality to chat with kdesu_stub.
 */

class KDESU_EXPORT StubProcess: public PtyProcess
{
public:
    StubProcess();
    ~StubProcess();

    /**
     * Set the command.
     */
    void setCommand(const QByteArray &command);

    /**
     * Set the target user.
     */
    void setUser(const QByteArray &user);

    /**
     * Set to "X only mode": Sycoca is not built and kdeinit is not launched.
     */
    void setXOnly(bool xonly);

    /**
     * Set the priority of the process. The priority value must be between 0
     * and 100, 0 being the lowest priority. This value is mapped to the
     * scheduler and system dependent priority range of the OS.
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
    void setScheduler(int sched);

protected:

    /**
     * Exchange all parameters with kdesu_stub.
     */
    int ConverseStub(int check);

    /**
     * This virtual function can be overloaded when special behavior is
     * desired. By default, it returns the value returned by KCookie.
     */
    virtual QByteArray display();
#ifdef Q_WS_X11
    /**
     * See display.
     */
    virtual QByteArray displayAuth();
#endif
    bool m_bXOnly;
    int m_Priority;
    int  m_Scheduler;
    QByteArray m_Command;
    QByteArray m_User;
    KDESuPrivate::KCookie *m_pCookie;

private:
    QByteArray commaSeparatedList(const QList<QByteArray> &);

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class StubProcessPrivate;
    StubProcessPrivate * const d;
};

}

#endif // __Stub_h_Included__
