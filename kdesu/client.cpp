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

#include <kdebug.h>
#include <kstddirs.h>

#include "client.h"

#ifdef __GNUC__
#define ID __PRETTY_FUNCTION__ << ": "
#else
#define ID "KDEsuClient: "
#endif

#ifndef SUN_LEN
#define SUN_LEN(ptr) ((ksize_t) (((struct sockaddr_un *) 0)->sun_path) \
	             + strlen ((ptr)->sun_path))   
#endif

KDEsuClient::KDEsuClient()
{
    sockfd = -1;
    char *dpy = getenv("DISPLAY");
    if (dpy == 0L) 
    {
	kdWarning(900) << ID << "$DISPLAY is not set\n";
	return;
    }
    sock.sprintf("/tmp/kdesud_%d_%s", (int) getuid(), dpy);
    connect();
}


KDEsuClient::~KDEsuClient()
{
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
	kdWarning(900) << ID << "socket(): " << perror << "\n";
	return -1;
    }
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, sock);

    if (::connect(sockfd, (struct sockaddr *) &addr, SUN_LEN(&addr)) < 0) 
    {
	kdWarning(900) << ID << "connect():" << perror << "\n";
	close(sockfd); sockfd = -1;
	return -1;
    }
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
	kdWarning(900) << ID << "no reply from daemon\n";
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
    cmd += QCString().setNum(timeout);
    cmd += " ";
    cmd += escape(group);
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

int KDEsuClient::ping()
{
    return command("PING\n");
}

int KDEsuClient::stopServer()
{
    return command("STOP\n");
}

bool KDEsuClient::isServerSGID()
{
    QString daemon = KStandardDirs::findExe("kdesud");
    if (daemon.isEmpty())
    {
	kdWarning(900) << ID << "daemon not found\n";
	return false;
    }

    struct stat sbuf;
    if (stat(daemon.latin1(), &sbuf) < 0) 
    {
	kdWarning(900) << ID << "stat(): " << perror << "\n";
	return false;
    } 
    return (sbuf.st_mode & S_ISGID);
}

int KDEsuClient::startServer()
{
    if (!isServerSGID())
	kdWarning(900) << ID << "kdesud not setgid!\n";

    // kdesud only forks to the background after it is accepting
    // connections.
    int ret = system("kdesud");
    connect();
    return ret;
}
