/* vi: ts=8 sts=4 sw=4
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

#include "stub.h"

#include <kdesu/kdesu_export.h>

namespace KDESu {

/** \class SshProcess ssh.h kdesu/ssh.h
 * Executes a remote command, using ssh.
 */

class KDESU_EXPORT SshProcess: public StubProcess
{
public:
    explicit SshProcess(const QByteArray &host = QByteArray(),
                        const QByteArray &user = QByteArray(),
                        const QByteArray &command = QByteArray());
    ~SshProcess();

    enum Errors { SshNotFound=1, SshNeedsPassword, SshIncorrectPassword };

    /**
     * Sets the target host.
     */
    void setHost(const QByteArray &host);

    /**
     * Sets the localtion of the remote stub.
     */
    void setStub(const QByteArray &stub);

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

    QByteArray prompt() const;
    QByteArray error() const;

protected:
    virtual QByteArray display();
    virtual QByteArray displayAuth();

private:
    int ConverseSsh(const char *password, int check);

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class SshProcessPrivate;
    SshProcessPrivate * const d;
};

}

#endif
