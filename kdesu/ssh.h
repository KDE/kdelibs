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
 * Executes a remote command, using ssh.
 */

class SshProcess: public StubProcess
{
public:
    SshProcess(QCString host=0, QCString user=0, QCString command=0);
    ~SshProcess();

    enum Errors { SshNotFound=1, SshNeedsPassword, SshIncorrectPassword };

    /** Sets the target host. */
    void setHost(QCString host) { m_Host = host; }

    /** Sets the localtion of the remote stub. */
    void setStub(QCString stub);

    /** 
     * Checks if the current user@host needs a password. 
     * @return The prompt for the password if a password is required. A null
     * string otherwise.
     */
    int checkNeedPassword();

    /**
     * Checks if the stub is installed and if the password is correct.
     * @return Zero if everything is correct, nonzero otherwise.
     */
    int checkInstall(const char *password);

    /** Executes the command. */
    int exec(const char *password, int check=0);

    QCString prompt() { return m_Prompt; }
    QCString error() { return d->m_Error; }

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

    struct SshProcessPrivate
    {
	QCString m_Error;
	QCString m_Stub;
    };

    SshProcessPrivate *d;
};

#endif
