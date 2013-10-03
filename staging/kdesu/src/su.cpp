/*
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * Sudo support added by Jonathan Riddell <jriddell@ ubuntu.com>
 * Copyright (C) 2005 Canonical Ltd  // krazy:exclude=copyright (no email)
 *
 * This is free software; you can use this library under the GNU Library
 * General Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 *
 * su.cpp: Execute a program as another user with "class SuProcess".
 */

#include "su.h"
#include "kcookie.h"

#include <QtCore/QFile>
#include <QDebug>
#include <QStandardPaths>
#include <qplatformdefs.h>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <kuser.h>

#ifndef __PATH_SU
#define __PATH_SU "false"
#endif

#ifndef __PATH_SUDO
#define __PATH_SUDO "false"
#endif

#ifdef KDESU_USE_SUDO_DEFAULT
#  define DEFAULT_SUPER_USER_COMMAND "sudo"
#else
#  define DEFAULT_SUPER_USER_COMMAND "su"
#endif

namespace KDESu {
using namespace KDESuPrivate;

class SuProcess::SuProcessPrivate
{
public:
    QString superUserCommand;
};

SuProcess::SuProcess(const QByteArray &user, const QByteArray &command)
    : d(new SuProcessPrivate)
{
    m_user = user;
    m_command = command;

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group(config, "super-user-command");
    d->superUserCommand = group.readEntry("super-user-command", DEFAULT_SUPER_USER_COMMAND);

    if (d->superUserCommand != "sudo" && d->superUserCommand != "su") {
        qWarning() << "unknown super user command.";
        d->superUserCommand = DEFAULT_SUPER_USER_COMMAND;
    }
}

SuProcess::~SuProcess()
{
    delete d;
}

QString SuProcess::superUserCommand()
{
    return d->superUserCommand;
}

bool SuProcess::useUsersOwnPassword()
{
    if (superUserCommand() == "sudo" && m_user == "root") {
        return true;
    }

    KUser user;
    return user.loginName() == m_user;
}

int SuProcess::checkInstall(const char *password)
{
    return exec(password, Install);
}

int SuProcess::checkNeedPassword()
{
    return exec(0L, NeedPassword);
}

/*
* Execute a command with su(1).
*/
int SuProcess::exec(const char *password, int check)
{
    if (check) {
        setTerminal(true);
    }

    // since user may change after constructor (due to setUser())
    // we need to override sudo with su for non-root here
    if (m_user != "root") {
        d->superUserCommand = "su";
    }

    QList<QByteArray> args;
    if (d->superUserCommand == "sudo") {
        args += "-u";
    }

    if (m_scheduler != SchedNormal || m_priority > 50) {
        args += "root";
    } else {
        args += m_user;
    }

    if (d->superUserCommand == "su") {
        args += "-c";
    }
    args += QByteArray(LIBEXEC_INSTALL_DIR) + "/kdesu_stub";
    args += "-"; // krazy:exclude=doublequote_chars (QList, not QString)

    QByteArray command;
    if (d->superUserCommand == "sudo") {
        command = __PATH_SUDO;
    } else {
        command = __PATH_SU;
    }

    if (QT_ACCESS(command.constData(), X_OK) != 0) {
        command = QFile::encodeName(QStandardPaths::findExecutable(d->superUserCommand.toLatin1()));
        if (command.isEmpty()) {
            return check ? SuNotFound : -1;
        }
    }

    if (StubProcess::exec(command, args) < 0) {
        return check ? SuNotFound : -1;
    }

    SuErrors ret = (SuErrors)converseSU(password);

    if (ret == error) {
        if (!check) {
            qCritical() << "[" << __FILE__ << ":" << __LINE__ << "] " << "Conversation with su failed.";
        }
        return ret;
    }
    if (check == NeedPassword) {
        if (ret == killme) {
            if (d->superUserCommand == "sudo") {
                // sudo can not be killed, just return
                return ret;
            }
            if (kill(m_pid, SIGKILL) < 0) {
                //FIXME SIGKILL doesn't work for sudo,
                //why is this different from su?
                //A: because sudo runs as root. Perhaps we could write a Ctrl+C to its stdin, instead?
                ret = error;
            } else {
                int iret = waitForChild();
                if (iret < 0) {
                    ret = error;
                }
            }
        }
        return ret;
    }

    if (m_erase && password) {
        memset(const_cast<char *>(password), 0, qstrlen(password));
    }

    if (ret != ok) {
        kill(m_pid, SIGKILL);
        if (d->superUserCommand != "sudo") {
            waitForChild();
        }
        return SuIncorrectPassword;
    }

    int iret = converseStub(check);
    if (iret < 0) {
        if (!check) {
            qCritical() << "[" << __FILE__ << ":" << __LINE__ << "] " << "Conversation with kdesu_stub failed.";
        }
        return iret;
    } else if (iret == 1) {
        kill(m_pid, SIGKILL);
        waitForChild();
        return SuIncorrectPassword;
    }

    if (check == Install) {
        waitForChild();
        return 0;
    }

    iret = waitForChild();
    return iret;
}

/*
* Conversation with su: feed the password.
* Return values: -1 = error, 0 = ok, 1 = kill me, 2 not authorized
*/
int SuProcess::converseSU(const char *password)
{
    enum { WaitForPrompt, CheckStar, HandleStub } state = WaitForPrompt;
    int colon;
    unsigned i, j;

    QByteArray line;
    while (true) {
        line = readLine();
        if (line.isNull()) {
            return ( state == HandleStub ? notauthorized : error);
        }

        if (line == "kdesu_stub") {
            unreadLine(line);
            return ok;
        }

        switch (state) {
        case WaitForPrompt: {
            if (waitMS(fd(), 100) > 0) {
                // There is more output available, so this line
                // couldn't have been a password prompt (the definition
                // of prompt being that  there's a line of output followed
                // by a colon, and then the process waits).
                continue;
            }

            const uint len = line.length();
            // Match "Password: " with the regex ^[^:]+:[\w]*$.
            for (i = 0, j = 0, colon = 0; i < len; ++i) {
                if (line[i] == ':') {
                    j = i; colon++;
                    continue;
                }
                if (!isspace(line[i])) {
                    j++;
                }
            }
            if (colon == 1 && line[j] == ':') {
                if (password == 0L) {
                    return killme;
                }
                if (waitSlave()) {
                    return error;
                }
                write(fd(), password, strlen(password));
                write(fd(), "\n", 1);
                state = CheckStar;
            }
            break;
        }
        //////////////////////////////////////////////////////////////////////////
        case CheckStar: {
            QByteArray s = line.trimmed();
            if (s.isEmpty()) {
                state = HandleStub;
                break;
            }
            const uint len = line.length();
            for (i = 0; i < len; ++i) {
                if (s[i] != '*') {
                    return error;
                }
            }
            state = HandleStub;
            break;
        }
        //////////////////////////////////////////////////////////////////////////
        case HandleStub:
            break;
        //////////////////////////////////////////////////////////////////////////
        } // end switch
    } // end while (true)
    return ok;
}

void SuProcess::virtual_hook(int id, void *data)
{
    StubProcess::virtual_hook(id, data);
}

} // namespace KDESu
