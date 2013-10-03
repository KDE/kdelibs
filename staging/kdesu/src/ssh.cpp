/*
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *
 * This is free software; you can use this library under the GNU Library
 * General Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 *
 * ssh.cpp: Execute a program on a remote machine using ssh.
 */

#include "ssh.h"
#include "kcookie_p.h"

#include <unistd.h>
#include <signal.h>

#include <QDebug>

extern int kdesuDebugArea();

namespace KDESu {

using namespace KDESuPrivate;

class SshProcess::SshProcessPrivate
{
public:
    SshProcessPrivate(const QByteArray &host)
        : host(host)
        , stub("kdesu_stub")
    {}
    QByteArray prompt;
    QByteArray host;
    QByteArray error;
    QByteArray stub;
};


SshProcess::SshProcess(const QByteArray &host, const QByteArray &user, const QByteArray &command)
    : d(new SshProcessPrivate(host))
{
    m_user = user;
    m_command = command;
    srand(time(0L));
}

SshProcess::~SshProcess()
{
    delete d;
}

void SshProcess::setHost(const QByteArray &host)
{
    d->host = host;
}

void SshProcess::setStub(const QByteArray &stub)
{
    d->stub = stub;
}

int SshProcess::checkInstall(const char *password)
{
    return exec(password, 1);
}

int SshProcess::checkNeedPassword()
{
    return exec(0L, 2);
}

int SshProcess::exec(const char *password, int check)
{
    if (check) {
        setTerminal(true);
    }

    QList<QByteArray> args;
    args += "-l";
    args += m_user;
    args += "-o";
    args += "StrictHostKeyChecking=no";
    args += d->host;
    args += d->stub;

    if (StubProcess::exec("ssh", args) < 0) {
        return check ? SshNotFound : -1;
    }

    int ret = converseSsh(password, check);
    if (ret < 0) {
        if (!check) {
            qCritical() << "[" << __FILE__ << ":" << __LINE__ << "] " << "Conversation with ssh failed.";
        }
        return ret;
    }
    if (check == 2) {
        if (ret == 1) {
            kill(m_pid, SIGTERM);
            waitForChild();
        }
        return ret;
    }

    if (m_erase && password) {
        memset(const_cast<char *>(password), 0, qstrlen(password));
    }

    ret = converseStub(check);
    if (ret < 0) {
        if (!check) {
            qCritical() << "[" << __FILE__ << ":" << __LINE__ << "] " << "Conversation with kdesu_stub failed.";
        }
        return ret;
    }
    else if (ret == 1) {
        kill(m_pid, SIGTERM);
        waitForChild();
        ret = SshIncorrectPassword;
    }

    if (check == 1) {
        waitForChild();
        return 0;
    }

    setExitString("Waiting for forwarded connections to terminate");
    ret = waitForChild();
    return ret;
}

QByteArray SshProcess::prompt() const
{
    return d->prompt;
}

QByteArray SshProcess::error() const
{
    return d->error;
}

/*
* Conversation with ssh.
* If check is 0, this waits for either a "Password: " prompt,
* or the header of the stub. If a prompt is received, the password is
* written back. Used for running a command.
* If check is 1, operation is the same as 0 except that if a stub header is
* received, the stub is stopped with the "stop" command. This is used for
* checking a password.
* If check is 2, operation is the same as 1, except that no password is
* written. The prompt is saved to prompt. Used for checking the need for
* a password.
*/
int SshProcess::converseSsh(const char *password, int check)
{
    unsigned i, j, colon;

    QByteArray line;
    int state = 0;

    while (state < 2) {
        line = readLine();
        const uint len = line.length();
        if (line.isNull()) {
            return -1;
        }

        switch (state) {
        case 0:
            // Check for "kdesu_stub" header.
            if (line == "kdesu_stub") {
                unreadLine(line);
                return 0;
            }

            // Match "Password: " with the regex ^[^:]+:[\w]*$.
            for (i = 0, j = 0, colon = 0; i < len; ++i) {
                if (line[i] == ':') {
                    j = i;
                    colon++;
                    continue;
                }
                if (!isspace(line[i])) {
                    j++;
                }
            }
            if ((colon == 1) && (line[j] == ':')) {
                if (check == 2) {
                    d->prompt = line;
                    return SshNeedsPassword;
                }
                if (waitSlave()) {
                    return -1;
                }
                write(fd(), password, strlen(password));
                write(fd(), "\n", 1);
                state++;
                break;
            }

            // Warning/error message.
            d->error += line;
            d->error += '\n';
            if (m_terminal) {
                fprintf(stderr, "ssh: %s\n", line.constData());
            }
            break;

        case 1:
            if (line.isEmpty()) {
                state++;
                break;
            }
            return -1;
        }
    }
    return 0;
}

// Display redirection is handled by ssh natively.
QByteArray SshProcess::display()
{
    return "no";
}

QByteArray SshProcess::displayAuth()
{
    return "no";
}

void SshProcess::virtual_hook(int id, void *data)
{
    StubProcess::virtual_hook(id, data);
}

} // namespace KDESu
