/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
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

#include <config.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>

#include <qglobal.h>
#include <qcstring.h>
#include <qfile.h>

#include <kdebug.h>
#include <kstddirs.h>
#include <kapp.h>

#include "client.h"

class KDEsuClient::KDEsuClientPrivate {
public:
    QString daemon;
};

#ifndef SUN_LEN
#define SUN_LEN(ptr) ((socklen_t) (((struct sockaddr_un *) 0)->sun_path) \
	             + strlen ((ptr)->sun_path))
#endif

KDEsuClient::KDEsuClient()
{
    sockfd = -1;
    QCString display(getenv("DISPLAY"));
    if (display.isEmpty())
    {
        kdWarning(900) << k_lineinfo << "$DISPLAY is not set\n";
        return;
    }

    // strip the screen number from the display
    display.replace(QRegExp("\\.[0-9]+$"), "");

    sock = QFile::encodeName(locateLocal("socket", QString("kdesud_%1").arg(display)));
    d = new KDEsuClientPrivate;
    connect();
}


KDEsuClient::~KDEsuClient()
{
    delete d;
    if (sockfd >= 0)
	close(sockfd);
}

int KDEsuClient::connect()
{
    if (sockfd >= 0)
	close(sockfd);
    if (access(sock, R_OK|W_OK))
    {
	sockfd = -1;
	return -1;
    }

    sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
	kdWarning(900) << k_lineinfo << "socket(): " << perror << "\n";
	return -1;
    }
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, sock);

    if (::connect(sockfd, (struct sockaddr *) &addr, SUN_LEN(&addr)) < 0)
    {
        kdWarning(900) << k_lineinfo << "connect():" << perror << endl;
	close(sockfd); sockfd = -1;
	return -1;
    }

#if !defined(SO_PEERCRED) || !defined(HAVE_STRUCT_UCRED)
#ifdef __GNUC__
#warning "Using sloppy security checks"
#endif
    // We check the owner of the socket after we have connected.
    // If the socket was somehow not ours an attacker will be able
    // to delete it after we connect but shouldn't be able to
    // create a socket that is owned by us.
    struct stat s;
    if (lstat(sock, &s)!=0)
    {
        kdWarning(900) << "stat failed (" << sock << ")" << endl;
	close(sockfd); sockfd = -1;
	return -1;
    }
    if (s.st_uid != getuid())
    {
        kdWarning(900) << "socket not owned by me! socket uid = " << s.st_uid << endl;
	close(sockfd); sockfd = -1;
	return -1;
    }
    if (!S_ISSOCK(s.st_mode))
    {
        kdWarning(900) << "socket is not a socket (" << sock << ")" << endl;
	close(sockfd); sockfd = -1;
	return -1;
    }
#else
    struct ucred cred;
    socklen_t siz = sizeof(cred);

    // Security: if socket exists, we must own it
    if (getsockopt(sockfd, SOL_SOCKET, SO_PEERCRED, &cred, &siz) == 0)
    {
        if (cred.uid != getuid())
        {
            kdWarning(900) << "socket not owned by me! socket uid = " << cred.uid << endl;
            close(sockfd); sockfd = -1;
            return -1;
        }
    }
#endif

    return 0;
}

QCString KDEsuClient::escape(QCString str)
{
    QCString copy = str;
    int n = 0;
    while ((n = copy.find("\\", n)) != -1)
    {
        copy.insert(n, '\\');
        n += 2;
    }
    n = 0;
    while ((n = copy.find("\"", n)) != -1)
    {
        copy.insert(n, '\\');
        n += 2;
    }
    copy.prepend("\"");
    copy.append("\"");
    return copy;
}

int KDEsuClient::command(QCString cmd, QCString *result)
{
    if (sockfd < 0)
	return -1;

    if (send(sockfd, cmd, cmd.length(), 0) != (int) cmd.length())
	return -1;

    char buf[1024];
    int nbytes = recv(sockfd, buf, 1023, 0);
    if (nbytes <= 0)
    {
	kdWarning(900) << k_lineinfo << "no reply from daemon\n";
	return -1;
    }
    buf[nbytes] = '\000';

    QCString reply = buf;
    if (reply.left(2) != "OK")
	return -1;

    if (result)
	*result = reply.mid(3, reply.length()-4);
    return 0;
}

int KDEsuClient::setPass(const char *pass, int timeout)
{
    QCString cmd = "PASS ";
    cmd += escape(pass);
    cmd += " ";
    cmd += QCString().setNum(timeout);
    cmd += "\n";
    return command(cmd);
}

int KDEsuClient::exec(QCString prog, QCString user)
{
    QCString cmd;
    cmd = "EXEC ";
    cmd += escape(prog);
    cmd += " ";
    cmd += escape(user);
    cmd += "\n";
    return command(cmd);
}

int KDEsuClient::setHost(QCString host)
{
    QCString cmd = "HOST ";
    cmd += escape(host);
    cmd += "\n";
    return command(cmd);
}

int KDEsuClient::setPriority(int prio)
{
    QCString cmd;
    cmd.sprintf("PRIO %d\n", prio);
    return command(cmd);
}

int KDEsuClient::setScheduler(int sched)
{
    QCString cmd;
    cmd.sprintf("SCHD %d\n", sched);
    return command(cmd);
}

int KDEsuClient::delCommand(QCString key, QCString user)
{
    QCString cmd = "DEL ";
    cmd += escape(key);
    cmd += " ";
    cmd += escape(user);
    cmd += "\n";
    return command(cmd);
}
int KDEsuClient::setVar(QCString key, QCString value, int timeout,
                        QCString group)
{
    QCString cmd = "SET ";
    cmd += escape(key);
    cmd += " ";
    cmd += escape(value);
    cmd += " ";
    cmd += escape(group);
    cmd += " ";
    cmd += QCString().setNum(timeout);
    cmd += "\n";
    return command(cmd);
}

QCString KDEsuClient::getVar(QCString key)
{
    QCString cmd = "GET ";
    cmd += escape(key);
    cmd += "\n";
    QCString reply;
    command(cmd, &reply);
    return reply;
}

QValueList<QCString> KDEsuClient::getKeys(QCString group)
{
    QCString cmd = "GETK ";
    cmd += escape(group);
    cmd += "\n";
    QCString reply;
    command(cmd, &reply);
    int index=0, pos;
    QValueList<QCString> list;
    if( !reply.isEmpty() )
    {
        // kdDebug(900) << "Found a matching entry: " << reply << endl;
        while (1)
        {
            pos = reply.find( '\007', index );
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

bool KDEsuClient::findGroup(QCString group)
{
    QCString cmd = "CHKG ";
    cmd += escape(group);
    cmd += "\n";
    if( command(cmd) == -1 )
        return false;
    return true;
}

int KDEsuClient::delVar(QCString key)
{
    QCString cmd = "DELV ";
    cmd += escape(key);
    cmd += "\n";
    return command(cmd);
}

int KDEsuClient::delGroup(QCString group)
{
    QCString cmd = "DELG ";
    cmd += escape(group);
    cmd += "\n";
    return command(cmd);
}

int KDEsuClient::delVars(QCString special_key)
{
    QCString cmd = "DELS ";
    cmd += escape(special_key);
    cmd += "\n";
    return command(cmd);
}

int KDEsuClient::ping()
{
    return command("PING\n");
}

int KDEsuClient::stopServer()
{
    return command("STOP\n");
}

static QString findDaemon()
{
    QString daemon = locate("bin", "kdesud");
    if (daemon.isEmpty()) // if not in KDEDIRS, rely on PATH
	daemon = KStandardDirs::findExe("kdesud");

    if (daemon.isEmpty())
    {
	kdWarning(900) << k_lineinfo << "daemon not found\n";
    }
    return daemon;
}

bool KDEsuClient::isServerSGID()
{
    if (d->daemon.isEmpty())
       d->daemon = findDaemon();
    if (d->daemon.isEmpty())
       return false;
   
    struct stat sbuf;
    if (stat(QFile::encodeName(d->daemon), &sbuf) < 0)
    {
	kdWarning(900) << k_lineinfo << "stat(): " << perror << "\n";
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
	kdWarning(900) << k_lineinfo << "kdesud not setgid!\n";
    }

    // kdesud only forks to the background after it is accepting
    // connections.
    // We start it via kdeinit to make sure that it doesn't inherit
    // any fd's from the parent process.
    int ret = kapp->kdeinitExecWait(d->daemon);
    connect();
    return ret;
}
