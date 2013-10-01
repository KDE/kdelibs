/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * This is free software; you can use this library under the GNU Library
 * General Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 *
 * client.cpp: A client for kdesud.
 */

#include "client.h"

#include <config-kdesu.h>

#include <errno.h>

#include <sys/socket.h>
#include <sys/un.h>

#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QStandardPaths>
#include <qplatformdefs.h>
#include <QDebug>

#include <ktoolinvocation.h>

extern int kdesuDebugArea();

namespace KDESu {

class KDEsuClient::KDEsuClientPrivate {
public:
    KDEsuClientPrivate() : sockfd(-1) {}
    QString daemon;
    int sockfd;
    QByteArray sock;
};

#ifndef SUN_LEN
#define SUN_LEN(ptr) ((QT_SOCKLEN_T) (((struct sockaddr_un *) 0)->sun_path) \
                     + strlen ((ptr)->sun_path))
#endif

KDEsuClient::KDEsuClient()
    :d(new KDEsuClientPrivate)
{
#if HAVE_X11
    QString display = QString::fromLocal8Bit(qgetenv("DISPLAY"));
    if (display.isEmpty())
    {
        qWarning() << "[" << __FILE__ << ":" << __LINE__ << "] " << "$DISPLAY is not set.";
        return;
    }

    // strip the screen number from the display
    display.remove(QRegExp(QStringLiteral("\\.[0-9]+$")));
#else
    QString display = QStringLiteral("NODISPLAY");
#endif

    d->sock = QFile::encodeName( QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation) + QStringLiteral("/kdesud_") + display);
    connect();
}


KDEsuClient::~KDEsuClient()
{
    if (d->sockfd >= 0)
        close(d->sockfd);
    delete d;
}

int KDEsuClient::connect()
{
    if (d->sockfd >= 0)
        close(d->sockfd);
    if (access(d->sock.constData(), R_OK|W_OK))
    {
        d->sockfd = -1;
        return -1;
    }

    d->sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (d->sockfd < 0)
    {
        qWarning() << "[" << __FILE__ << ":" << __LINE__ << "] " << "socket():" << strerror(errno);
        return -1;
    }
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, d->sock.constData());

    if (QT_SOCKET_CONNECT(d->sockfd, (struct sockaddr *) &addr, SUN_LEN(&addr)) < 0)
    {
        qWarning() << "[" << __FILE__ << ":" << __LINE__ << "] " << "connect():" << strerror(errno);
        close(d->sockfd); d->sockfd = -1;
        return -1;
    }

#if !defined(SO_PEERCRED) || ! HAVE_STRUCT_UCRED
# if HAVE_GETPEEREID
    uid_t euid;
    gid_t egid;
    // Security: if socket exists, we must own it
    if (getpeereid(d->sockfd, &euid, &egid) == 0)
    {
       if (euid != getuid())
       {
            qWarning() << "socket not owned by me! socket uid =" << euid;
            close(d->sockfd); d->sockfd = -1;
            return -1;
       }
    }
# else
#  ifdef __GNUC__
#   warning "Using sloppy security checks"
#  endif
    // We check the owner of the socket after we have connected.
    // If the socket was somehow not ours an attacker will be able
    // to delete it after we connect but shouldn't be able to
    // create a socket that is owned by us.
    QT_STATBUF s;
    if (QT_LSTAT(d->sock.constData(), &s)!=0)
    {
        qWarning() << "stat failed (" << d->sock << ")";
        close(d->sockfd); d->sockfd = -1;
        return -1;
    }
    if (s.st_uid != getuid())
    {
        qWarning() << "socket not owned by me! socket uid =" << s.st_uid;
        close(d->sockfd); d->sockfd = -1;
        return -1;
    }
    if (!S_ISSOCK(s.st_mode))
    {
        qWarning() << "socket is not a socket (" << d->sock << ")";
        close(d->sockfd); d->sockfd = -1;
        return -1;
    }
# endif
#else
    struct ucred cred;
    QT_SOCKLEN_T siz = sizeof(cred);

    // Security: if socket exists, we must own it
    if (getsockopt(d->sockfd, SOL_SOCKET, SO_PEERCRED, &cred, &siz) == 0)
    {
        if (cred.uid != getuid())
        {
            qWarning() << "socket not owned by me! socket uid =" << cred.uid;
            close(d->sockfd); d->sockfd = -1;
            return -1;
        }
    }
#endif

    return 0;
}

QByteArray KDEsuClient::escape(const QByteArray &str)
{
    QByteArray copy;
    copy.reserve(str.size() + 4);
    copy.append('"');
    for (int i = 0; i < str.size(); i++) {
        uchar c = str.at(i);
        if (c < 32) {
            copy.append('\\');
            copy.append('^');
            copy.append(c + '@');
        } else {
            if (c == '\\' || c == '"')
                copy.append('\\');
            copy.append(c);
        }
    }
    copy.append('"');
    return copy;
}

int KDEsuClient::command(const QByteArray &cmd, QByteArray *result)
{
    if (d->sockfd < 0)
        return -1;

    if (send(d->sockfd, cmd.constData(), cmd.length(), 0) != (int) cmd.length())
        return -1;

    char buf[1024];
    int nbytes = recv(d->sockfd, buf, 1023, 0);
    if (nbytes <= 0)
    {
        qWarning() << "[" << __FILE__ << ":" << __LINE__ << "] " << "no reply from daemon.";
        return -1;
    }
    buf[nbytes] = '\000';

    QByteArray reply = buf;
    if (reply.left(2) != "OK")
        return -1;

    if (result)
        *result = reply.mid(3, reply.length()-4);
    return 0;
}

int KDEsuClient::setPass(const char *pass, int timeout)
{
    QByteArray cmd = "PASS ";
    cmd += escape(pass);
    cmd += ' ';
    cmd += QByteArray().setNum(timeout);
    cmd += '\n';
    return command(cmd);
}

int KDEsuClient::exec(const QByteArray &prog, const QByteArray &user, const QByteArray &options, const QList<QByteArray> &env)
{
    QByteArray cmd;
    cmd = "EXEC ";
    cmd += escape(prog);
    cmd += ' ';
    cmd += escape(user);
    if (!options.isEmpty() || !env.isEmpty())
    {
       cmd += ' ';
       cmd += escape(options);
       for (int i = 0; i < env.count(); ++i)
       {
          cmd += ' ';
          cmd += escape(env.at(i));
       }
    }
    cmd += '\n';
    return command(cmd);
}

int KDEsuClient::setHost(const QByteArray &host)
{
    QByteArray cmd = "HOST ";
    cmd += escape(host);
    cmd += '\n';
    return command(cmd);
}

int KDEsuClient::setPriority(int prio)
{
    QByteArray cmd;
    cmd += "PRIO ";
    cmd += QByteArray::number(prio);
    cmd += '\n';
    return command(cmd);
}

int KDEsuClient::setScheduler(int sched)
{
    QByteArray cmd;
    cmd += "SCHD ";
    cmd += QByteArray::number(sched);
    cmd += '\n';
    return command(cmd);
}

int KDEsuClient::delCommand(const QByteArray &key, const QByteArray &user)
{
    QByteArray cmd = "DEL ";
    cmd += escape(key);
    cmd += ' ';
    cmd += escape(user);
    cmd += '\n';
    return command(cmd);
}
int KDEsuClient::setVar(const QByteArray &key, const QByteArray &value, int timeout,
                        const QByteArray &group)
{
    QByteArray cmd = "SET ";
    cmd += escape(key);
    cmd += ' ';
    cmd += escape(value);
    cmd += ' ';
    cmd += escape(group);
    cmd += ' ';
    cmd += QByteArray().setNum(timeout);
    cmd += '\n';
    return command(cmd);
}

QByteArray KDEsuClient::getVar(const QByteArray &key)
{
    QByteArray cmd = "GET ";
    cmd += escape(key);
    cmd += '\n';
    QByteArray reply;
    command(cmd, &reply);
    return reply;
}

QList<QByteArray> KDEsuClient::getKeys(const QByteArray &group)
{
    QByteArray cmd = "GETK ";
    cmd += escape(group);
    cmd += '\n';
    QByteArray reply;
    command(cmd, &reply);
    int index=0, pos;
    QList<QByteArray> list;
    if( !reply.isEmpty() )
    {
        // qDebug() << "Found a matching entry:" << reply;
        while (1)
        {
            pos = reply.indexOf( '\007', index );
            if( pos == -1 )
            {
                if( index == 0 )
                    list.append( reply );
                else
                    list.append( reply.mid(index) );
                break;
            }
            else
            {
                list.append( reply.mid(index, pos-index) );
            }
            index = pos+1;
        }
    }
    return list;
}

bool KDEsuClient::findGroup(const QByteArray &group)
{
    QByteArray cmd = "CHKG ";
    cmd += escape(group);
    cmd += '\n';
    if( command(cmd) == -1 )
        return false;
    return true;
}

int KDEsuClient::delVar(const QByteArray &key)
{
    QByteArray cmd = "DELV ";
    cmd += escape(key);
    cmd += '\n';
    return command(cmd);
}

int KDEsuClient::delGroup(const QByteArray &group)
{
    QByteArray cmd = "DELG ";
    cmd += escape(group);
    cmd += '\n';
    return command(cmd);
}

int KDEsuClient::delVars(const QByteArray &special_key)
{
    QByteArray cmd = "DELS ";
    cmd += escape(special_key);
    cmd += '\n';
    return command(cmd);
}

int KDEsuClient::ping()
{
    return command("PING\n");
}

int KDEsuClient::exitCode()
{
    QByteArray result;
    if (command("EXIT\n", &result) != 0)
       return -1;

    return result.toInt();
}

int KDEsuClient::stopServer()
{
    return command("STOP\n");
}

static QString findDaemon()
{
    QString daemon = QFile::decodeName(CMAKE_INSTALL_PREFIX "/" LIBEXEC_INSTALL_DIR "/kdesud");
    if (!QFile::exists(daemon)) { // if not in libexec, find it in PATH
        daemon = QStandardPaths::findExecutable(QStringLiteral("kdesud"));
        if (daemon.isEmpty()) {
            qWarning() << "kdesud daemon not found.";
        }
    }
    return daemon;
}

bool KDEsuClient::isServerSGID()
{
    if (d->daemon.isEmpty())
       d->daemon = findDaemon();
    if (d->daemon.isEmpty())
       return false;

    QT_STATBUF sbuf;
    if (QT_STAT(QFile::encodeName(d->daemon).constData(), &sbuf) < 0)
    {
        qWarning() << "[" << __FILE__ << ":" << __LINE__ << "] " << "stat():" << strerror(errno);
        return false;
    }
    return (sbuf.st_mode & S_ISGID);
}

int KDEsuClient::startServer()
{
    if (d->daemon.isEmpty())
        d->daemon = findDaemon();
    if (d->daemon.isEmpty())
        return -1;

    if (!isServerSGID()) {
        qWarning() << "[" << __FILE__ << ":" << __LINE__ << "] " << "kdesud not setgid!";
    }

    // kdesud only forks to the background after it is accepting
    // connections.
    // We start it via kdeinit to make sure that it doesn't inherit
    // any fd's from the parent process.
    int ret = KToolInvocation::kdeinitExecWait(d->daemon);
    connect();
    return ret;
}

}
