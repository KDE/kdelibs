/*
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * This is free software; you can use this library under the GNU Library
 * General Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 */

#ifndef KDESUSU_H
#define KDESUSU_H

#include <kdesu_export.h>

#include "stub.h"

namespace KDESu {

/** \class SuProcess su.h kdesu/su.h
 * Executes a command under elevated privileges, using su.
 */

class KDESU_EXPORT SuProcess : public StubProcess
{
public:
    enum Errors {
        SuNotFound = 1,
        SuNotAllowed,
        SuIncorrectPassword
    };

    /**
     * Executes the command. This will wait for the command to finish.
     */
    enum checkMode {
        NoCheck = 0,
        Install = 1,
        NeedPassword = 2
    };

    explicit SuProcess(const QByteArray &user = 0, const QByteArray &command = 0);
    ~SuProcess();

    int exec(const char *password, int check = NoCheck);

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

protected:
    virtual void virtual_hook(int id, void *data);

private:
    enum SuErrors {
        error = -1,
        ok = 0,
        killme = 1,
        notauthorized = 2
    };

    int converseSU(const char *password);

    class SuProcessPrivate;
    SuProcessPrivate * const d;
};

}

#endif //KDESUSU_H
