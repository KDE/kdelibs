/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * This is free software; you can use this library under the GNU Library
 * General Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 */

#ifndef __SU_h_Included__
#define __SU_h_Included__

#include <kdesu/kdesu_export.h>

#include "stub.h"


namespace KDESu {

/** \class SuProcess su.h kdesu/su.h
 * Executes a command under elevated privileges, using su.
 */

class KDESU_EXPORT SuProcess: public StubProcess
{
public:
    explicit SuProcess(const QByteArray &user=0, const QByteArray &command=0);
    ~SuProcess();

    enum Errors { SuNotFound=1, SuNotAllowed, SuIncorrectPassword };

    /**
     * Executes the command. This will wait for the command to finish.
     */
    enum checkMode { NoCheck=0, Install=1, NeedPassword=2 } ;
    int exec(const char *password, int check=NoCheck);

    /**
     * Checks if the stub is installed and the password is correct.
     * @return Zero if everything is correct, nonzero otherwise.
     */
    int checkInstall(const char *password);

    /**
     * Checks if a password is needed.
     */
    int checkNeedPassword();

    /**
     * Checks what the default super user command is, e.g. sudo, su, etc
     * @return the default super user command
     */
    QString superUserCommand();

    /**
     * Checks whether or not the user's password is being asked for or another
     * user's password. Due to usage of systems such as sudo, even when attempting
     * to switch to another user one may need to enter their own password.
     */
    bool useUsersOwnPassword();

private:
    enum SuErrors { error=-1, ok=0, killme=1, notauthorized=2 } ;
    int ConverseSU(const char *password);

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class SuProcessPrivate;
    SuProcessPrivate * const d;
};

}

#endif
