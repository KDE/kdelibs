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


SuProcess::SuProcess(const QCString &user, const QCString &command)
{
    m_User = user;
    m_Command = command;
}


SuProcess::~SuProcess()
{
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
    if (check)
	setTerminal(true);

    QCStringList args;
    if ((m_Scheduler != SchedNormal) || (m_Priority > 50))
	args += "root";
    else
	args += m_User;
    args += "-c";
    args += QCString(__KDE_BINDIR) + "/kdesu_stub";
    args += "-";

    QCString command = __PATH_SU;
    if (::access(__PATH_SU, X_OK) != 0)
    {
       command = QFile::encodeName(KGlobal::dirs()->findExe("su"));
       if (command.isEmpty())
          return check ? SuNotFound : -1;
    }

    // kdDebug(900) << k_lineinfo << "Call StubProcess::exec()" << endl;
    if (StubProcess::exec(command, args) < 0)
    {
	return check ? SuNotFound : -1;
    }
    // kdDebug(900) << k_lineinfo << "Done StubProcess::exec()" << endl;
    
    SuErrors ret = (SuErrors) ConverseSU(password);
    // kdDebug(900) << k_lineinfo << "Conversation returned " << ret << endl;

    if (ret == error) 
    {
	if (!check)
	    kdError(900) << k_lineinfo << "Conversation with su failed\n";
	return ret;
    } 
    if (check == NeedPassword)
    {
	if (ret == killme)
	{
	    if (kill(m_Pid, SIGKILL) < 0) ret=error;
	    else 
	    {
		int iret = waitForChild();
		if (iret < 0) ret=error;
		else /* nothing */ {} ;
	    }
	}
	return ret;
    }

    if (m_bErase && password) 
    {
	char *ptr = const_cast<char *>(password);
	for (unsigned i=0; i<strlen(password); i++)
	    ptr[i] = '\000';
    }

    if (ret == notauthorized)
    {
	kill(m_Pid, SIGKILL);
	waitForChild();
	return SuIncorrectPassword;
    }

    int iret = ConverseStub(check);
    if (iret < 0)
    {
	if (!check)
	    kdError(900) << k_lineinfo << "Converstation with kdesu_stub failed\n";
	return iret;
    } else if (iret == 1)
    {
	kill(m_Pid, SIGKILL);
	waitForChild();
	return SuIncorrectPassword;
    }

    if (check == Install)
    {
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

int SuProcess::ConverseSU(const char *password)
{	
    enum { WaitForPrompt, CheckStar, HandleStub } state = WaitForPrompt;
    int colon;
    unsigned i, j;
    
    // kdDebug(900) << k_lineinfo << "ConverseSU starting." << endl;

    QCString line;
    while (true)
    {
	line = readLine(); 
	if (line.isNull())
	    return ( state == HandleStub ? notauthorized : error);
	kdDebug(900) << k_lineinfo << "Read line <" << line << ">" << endl;

	switch (state) 
	{
	case WaitForPrompt:
	    // In case no password is needed.
	    if (line == "kdesu_stub")
	    {
		unreadLine(line);
		return ok;
	    }

            while(waitMS(m_Fd,100)>0)
            {
		// There is more output available, so the previous line
		// couldn't have been a password prompt (the definition
		// of prompt being that  there's a line of output followed 
		// by a colon, and then the process waits).
		QCString more = readLine();
		if (more.isEmpty()) break;
		line = more;
		kdDebug(900) << k_lineinfo << "Read line <" << more << ">" << endl;
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
		    return killme;
		if (!checkPid(m_Pid))
		{
		    kdError(900) << "su has exited while waiting for pwd." << endl;
		    return error;
		}
		if ((WaitSlave() == 0) && checkPid(m_Pid))
		{
			write(m_Fd, password, strlen(password));
			write(m_Fd, "\n", 1);
			state=CheckStar;
		}
		else
		{
			return error;
		}
	    }
	    break;

	case CheckStar: 
	{
	    QCString s = line.stripWhiteSpace();
	    if (s.isEmpty()) 
	    {
		state=HandleStub;
		break;
	    }
	    for (i=0; i<s.length(); i++)
            {
		if (s[i] != '*')
		    return error;
	    }
	    state=HandleStub;
	    break;
	}

	case HandleStub:
	    // Read till we get "kdesu_stub"
	    if (line == "kdesu_stub")
	    {
		unreadLine(line);
		return ok;
	    }
	    break;
	}
    }
    return ok;
}

void SuProcess::virtual_hook( int id, void* data )
{ StubProcess::virtual_hook( id, data ); }
