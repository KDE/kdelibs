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
 * su.cpp: Execute a program as another user with "class SuProcess".
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <qglobal.h>
#include <qcstring.h>
#include <qfile.h>

#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "su.h"
#include "kcookie.h"


#ifndef __PATH_SU
#define __PATH_SU "false"
#endif


SuProcess::SuProcess(QCString user, QCString command)
{
    m_User = user;
    m_Command = command;
}


SuProcess::~SuProcess()
{
}

int SuProcess::checkInstall(const char *password)
{
    return exec(password, 1);
}

int SuProcess::checkNeedPassword()
{
    return exec(0L, 2);
}

/*
 * Execute a command with su(1).
 */

int SuProcess::exec(const char *password, int check)
{
    if (check)
	setTerminal(true);

    QCStringList args;
    if ((m_Scheduler != SchedNormal) || (m_Priority > 50))
	args += "root";
    else
	args += m_User;
    args += "-c";
    args += QCString(__KDE_BINDIR) + "/kdesu_stub";

    if (StubProcess::exec(__PATH_SU, args) < 0)
    {
	return check ? SuNotFound : -1;
    }
    
    int ret = ConverseSU(password);
    if (ret < 0) 
    {
	if (!check)
	    kdError(900) << k_lineinfo << "Conversation with su failed\n";
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

    if (m_bErase) 
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
	return SuIncorrectPassword;
    }
    if (check == 1)
    {
	waitForChild();
	return 0;
    }
	
    // Notify the taskbar that an app has been started. 
    //QString suffix = i18n("(as %1)").arg(m_User);
    //notifyTaskbar(suffix);
    ret = waitForChild();
    return ret;
}

/*
 * Conversation with su: feed the password.
 * Return values: -1 = error, 0 = ok, 1 = kill me
 */

int SuProcess::ConverseSU(const char *password)
{	
    int colon, state = 0;
    unsigned i, j;

    QCString line;
    while (state < 2) 
    {
	line = readLine(); 
	if (line.isNull())
	    return -1;
	
	switch (state) 
	{
	case 0:
	    // In case no password is needed.
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
		if (password == 0L)
		    return 1;
		WaitSlave();
		write(m_Fd, password, strlen(password));
		write(m_Fd, "\n", 1);
		state++;
	    }
	    break;

	case 1:
	    if (line.stripWhiteSpace().isEmpty()) 
	    {
		state++;
		break;
	    }
	    QCString s=line.stripWhiteSpace();
	    for (i=0; i<s.length(); i++)
            {
		if (s[i] != '*')
		    return -1;
	    }
	    state++;
	    break;
	}
    }
    return 0;
}


QCStringList SuProcess::dcopServer()
{
    if (!m_bXOnly) 
	return StubProcess::dcopServer();

    QCStringList lst;
    lst += "no";
    return lst;
}

