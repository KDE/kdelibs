/* vi: ts=8 sts=4 sw=4
 *
 * $Id: ssh.h 433124 2005-07-09 19:01:03Z adridg $
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

#include <kdelibs_export.h>

/**
 * Executes a remote command, using ssh.
 */

class KDESU_EXPORT SshProcess: public StubProcess
{
public:
    SshProcess(const QCString &host=0, const QCString &user=0, const QCString &command=0);
    ~SshProcess();

    enum Errors { SshNotFound=1, SshNeedsPassword, SshIncorrectPassword };

    /**
     * Sets the target host.
     */
    void setHost(const QCString &host) { m_Host = host; }

    /**
     * Sets the localtion of the remote stub.
     */
    void setStub(const QCString &stub);

    /** 
     * Checks if the current user\@host needs a password. 
     * @return The prompt for the password if a password is required. A null
     * string otherwise.
     *
     * @todo The return doc is so obviously wrong that the C code needs to be checked.
     */
    int checkNeedPassword();

    /**
     * Checks if the stub is installed and if the password is correct.
     * @return Zero if everything is correct, nonzero otherwise.
     */
    int checkInstall(const char *password);

    /**
     * Executes the command.
     */
    int exec(const char *password, int check=0);

    QCString prompt() { return m_Prompt; }
    QCString error() { return m_Error; }

protected:
    virtual QCString display();
    virtual QCString displayAuth();
    virtual QCString dcopServer();

private:
    QCString dcopForward();
    int ConverseSsh(const char *password, int check);

    int m_dcopPort;
    int  m_dcopSrv;
    QCString m_Prompt;
    QCString m_Host;
    QCString m_Error;
    QCString m_Stub;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class SshProcessPrivate;
    SshProcessPrivate *d;
};

#endif
