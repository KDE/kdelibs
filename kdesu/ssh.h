/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 * 
 * This is free software; you can use this library under the GNU Library 
 * General Public License, version 2. See the file "COPYING.LIB" for the 
 * exact licensing terms.
 */

#ifndef __SSH_h_Included__
#define __SSH_h_Included__

#include <qcstring.h>

#include "stub.h"

/**
 * Execute a remote command, using ssh.
 */

class SshProcess: public StubProcess
{
public:
    SshProcess(QCString host=0, QCString user=0, QCString command=0);
    ~SshProcess();

    /** Set the target host. */
    void setHost(QCString host) { m_Host = host; }

    /** 
     * Check if the current user@host needs a password. 
     * @return The prompt for the password if a password is required. A null
     * string otherwise.
     */
    QCString checkNeedPassword();

    /**
     * Check if the stub is installed and if the password is correct.
     * @return Zero if everything is correct, nonzero otherwise.
     */
    int checkInstall(const char *password);

    /** Execute the command. */
    int exec(const char *password, int check=0);

protected:
    virtual QCString display();
    virtual QCString displayAuth();
    virtual QCStringList dcopServer();
    virtual QCStringList dcopAuth();
    virtual QCStringList iceAuth();

private:
    QCString dcopForward();
    int ConverseSsh(const char *password, int check);

    int m_dcopPort, m_dcopSrv;
    QCString m_Prompt, m_Host;

    class SshProcessPrivate;
    SshProcessPrivate *d;
};

#endif
