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
#define ID __PRETTY_FUNCTION__
#else
#define ID "KDEsuClient"
#endif

#ifndef SUN_LEN
#define SUN_LEN(ptr) ((ksize_t) (((struct sockaddr_un *) 0)->sun_path) \
	             + strlen ((ptr)->sun_path))   
#endif

KDEsuClient::KDEsuClient()
{
    sockfd = -1;
    char *dpy = getenv("DISPLAY");
    if (dpy == 0L) {
	kDebugWarning("%s: $DISPLAY is not set", ID);
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
    if (access(sock, R_OK|W_OK)) {
	sockfd = -1;
	return -1;
    }

    sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
	kDebugWarning("%s: socket(): %m", ID);
	return -1;
    }
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, sock);

    if (::connect(sockfd, (struct sockaddr *) &addr, SUN_LEN(&addr)) < 0) {
	kDebugWarning("%s: connect(): %m", ID);
	close(sockfd); sockfd = -1;
	return -1;
    }
    return 0;
}


QCString KDEsuClient::escape(QCString str)
{
    QCString copy = str;

    int n = 0;
    while ((n = copy.find("\\", n)) != -1) {
	copy.insert(n, '\\');
	n += 2;
    }
    n = 0;
    while ((n = copy.find("\"", n)) != -1) {
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
    
    char buf[200];
    int nbytes = recv(sockfd, buf, 199, 0);
    if (nbytes <= 0) {
	kDebugWarning("%s: no reply from daemon", ID);
	return -1;
    }
    buf[nbytes] = '\000';

    QCString reply = buf;
    if (reply.left(2) != "OK") 
	return -1;

    if (result)
	*result = reply.mid(2).stripWhiteSpace();
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


int KDEsuClient::exec(QCString key)
{
    QCString cmd;
    cmd = "EXEC ";
    cmd += escape(key);
    cmd += "\n";
    return command(cmd);
}


int KDEsuClient::setUser(QCString user)
{
    QCString cmd = "USER ";
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


int KDEsuClient::delCommand(QCString key)
{
    QCString cmd = "DEL ";
    cmd += escape(key);
    cmd += "\n";
    return command(cmd);
}


int KDEsuClient::setVar(QCString key, QCString value)
{
    QCString cmd = "SET ";
    cmd += escape(key);
    cmd += " ";
    cmd += escape(value);
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


int KDEsuClient::ping()
{
    return command("PING\n");
}


int KDEsuClient::stopServer()
{
    return command("STOP\n");
}

int KDEsuClient::startServer()
{
    QString daemon = KStandardDirs::findExe("kdesud");
    if (daemon.isEmpty())
	return -1;

    struct stat sbuf;
    if (stat(daemon.latin1(), &sbuf) < 0) {
	kDebugWarning("%s: stat(): %m", ID);
	return -1;
    }
    if (!(sbuf.st_mode & S_ISGID)) {
	kDebugWarning("%s: kdesud not setgid: not using it", ID);
	return -1;
    }

    return system("kdesud");
}
