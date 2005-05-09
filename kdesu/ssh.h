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

#include <q3cstring.h>

#include "stub.h"

#include <kdelibs_export.h>

/**
 * Executes a remote command, using ssh.
 */

class KDESU_EXPORT SshProcess: public StubProcess
{
public:
    SshProcess(const Q3CString &host=0, const Q3CString &user=0, const Q3CString &command=0);
    ~SshProcess();

    enum Errors { SshNotFound=1, SshNeedsPassword, SshIncorrectPassword };

    /**
     * Sets the target host.
     */
    void setHost(const Q3CString &host) { m_Host = host; }

    /**
     * Sets the localtion of the remote stub.
     */
    void setStub(const Q3CString &stub);

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

    /**
     * Executes the command.
     */
    int exec(const char *password, int check=0);

    Q3CString prompt() { return m_Prompt; }
    Q3CString error() { return m_Error; }

protected:
    virtual Q3CString display();
    virtual Q3CString displayAuth();
    virtual Q3CString dcopServer();

private:
    Q3CString dcopForward();
    int ConverseSsh(const char *password, int check);

    int m_dcopPort;
    int  m_dcopSrv;
    Q3CString m_Prompt;
    Q3CString m_Host;
    Q3CString m_Error;
    Q3CString m_Stub;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class SshProcessPrivate;
    SshProcessPrivate *d;
};

#endif
