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
 *
 * ssh.cpp: Execute a program on a remote machine using ssh.
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <qglobal.h>
#include <qcstring.h>

#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "ssh.h"
#include "kcookie.h"


SshProcess::SshProcess(const QCString &host, const QCString &user, const QCString &command)
{
    m_Host = host;
    m_User = user;
    m_Command = command;
    m_Stub = "kdesu_stub";
    srand(time(0L));
}


SshProcess::~SshProcess()
{
}


void SshProcess::setStub(const QCString &stub)
{
    m_Stub = stub;
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
    if (check)
	setTerminal(true);

    QCStringList args;
    args += "-l"; args += m_User;
    args += "-o"; args += "StrictHostKeyChecking=no";
    args += m_Host; args += m_Stub;

    if (StubProcess::exec("ssh", args) < 0)
    {
	return check ? SshNotFound : -1;
    }

    int ret = ConverseSsh(password, check);
    if (ret < 0)
    {
	if (!check)
	    kdError(900) << k_lineinfo << "Conversation with ssh failed\n";
	return ret;
    }
    if (check == 2)
    {
	if (ret == 1)
	{
	    kill(m_Pid, SIGTERM);
	    waitForChild();
	}
	return ret;
    }

    if (m_bErase && password)
    {
	char *ptr = const_cast<char *>(password);
	for (unsigned i=0; i<strlen(password); i++)
	    ptr[i] = '\000';
    }

    ret = ConverseStub(check);
    if (ret < 0)
    {
	if (!check)
	    kdError(900) << k_lineinfo << "Converstation with kdesu_stub failed\n";
	return ret;
    } else if (ret == 1)
    {
	kill(m_Pid, SIGTERM);
	waitForChild();
	ret = SshIncorrectPassword;
    }
    if (check == 1)
    {
	waitForChild();
	return 0;
    }

    // Notify the taskbar that an app has been started.
    QString suffix = i18n("(as %1@%2)").arg(m_User).arg(m_Host);
    notifyTaskbar(suffix);

    setExitString("Waiting for forwarded connections to terminate");
    ret = waitForChild();
    return ret;
}

/*
 * Create a port forwarding for DCOP. For the remote port, we take a pseudo
 * random number between 10k and 50k. This is not ok, of course, but I see
 * no other way. There is, afaik, no security issue involved here. If the port
 * happens to be occupied, ssh will refuse to start.
 *
 * 14/SEP/2000: DCOP forwarding is not used anymore.
 */

QCString SshProcess::dcopForward()
{
    QCString result;

    setDcopTransport("tcp");

    QCString srv = StubProcess::dcopServer();
    if (srv.isEmpty())
       return result;

    int i = srv.find('/');
    if (i == -1)
       return result;
    if (srv.left(i) != "tcp")
       return result;
    int j = srv.find(':', ++i);
    if (j == -1)
       return result;
    QCString host = srv.mid(i, j-i);
    bool ok;
    int port = srv.mid(++j).toInt(&ok);
    if (!ok)
       return result;

    m_dcopPort = 10000 + (int) ((40000.0 * rand()) / (1.0 + RAND_MAX));
    result.sprintf("%d:%s:%d", m_dcopPort, host.data(), port);
    return result;
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
 * written. The prompt is saved to m_Prompt. Used for checking the need for
 * a password.
 */

int SshProcess::ConverseSsh(const char *password, int check)
{
    unsigned i, j, colon;

    QCString line;
    int state = 0;

    while (state < 2)
    {
	line = readLine();
	if (line.isNull())
	    return -1;

	switch (state) {
	case 0:
	    // Check for "kdesu_stub" header.
	    if (line == "kdesu_stub")
	    {
		unreadLine(line);
		return 0;
	    }

	    // Match "Password: " with the regex ^[^:]+:[\w]*$.
	    for (i=0,j=0,colon=0; i<line.length(); i++)
	    {
		if (line[i] == ':')
		{
		    j = i; colon++;
		    continue;
		}
		if (!isspace(line[i]))
		    j++;
	    }
	    if ((colon == 1) && (line[j] == ':'))
	    {
		if (check == 2)
		{
		    m_Prompt = line;
		    return SshNeedsPassword;
		}
		WaitSlave();
		write(m_Fd, password, strlen(password));
		write(m_Fd, "\n", 1);
		state++;
		break;
	    }

	    // Warning/error message.
	    m_Error += line; m_Error += "\n";
	    if (m_bTerminal)
		fprintf(stderr, "ssh: %s\n", line.data());
	    break;

	case 1:
	    if (line.isEmpty())
	    {
		state++;
		break;
	    }
	    return -1;
	}
    }

    return 0;
}


// Display redirection is handled by ssh natively.
QCString SshProcess::display()
{
    return "no";
}


QCString SshProcess::displayAuth()
{
    return "no";
}


// Return the remote end of the forwarded connection.
QCString SshProcess::dcopServer()
{
    return QCString().sprintf("tcp/localhost:%d", m_dcopPort);
}

void SshProcess::virtual_hook( int id, void* data )
{ StubProcess::virtual_hook( id, data ); }
