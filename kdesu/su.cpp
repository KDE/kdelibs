/* vi: ts=8 sts=4 sw=4
*
* $Id: su.cpp 592751 2006-10-05 16:22:44Z coolo $
*
* This file is part of the KDE project, module kdesu.
* Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
* 
* Sudo support added by Jonathan Riddell <jriddell@ ubuntu.com>
* Copyright (C) 2005 Canonical Ltd
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

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "su.h"
#include "kcookie.h"


#ifndef __PATH_SU
#define __PATH_SU "false"
#endif

#ifndef __PATH_SUDO
#define __PATH_SUDO "false"
#endif

SuProcess::SuProcess(const QCString &user, const QCString &command)
{
    m_User = user;
    m_Command = command;

    KConfig* config = KGlobal::config();
    config->setGroup("super-user-command");
    superUserCommand = config->readEntry("super-user-command", DEFAULT_SUPER_USER_COMMAND);
    if ( superUserCommand != "sudo" && superUserCommand != "su" ) {
	kdWarning() << "unknown super user command" << endl;
	superUserCommand = "su";
    }
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

    // since user may change after constructor (due to setUser())
    // we need to override sudo with su for non-root here
    if (m_User != "root") {
        superUserCommand = "su";
    }

    QCStringList args;
    if (superUserCommand == "sudo") {
        args += "-u";
    }

#ifdef Q_OS_DARWIN
    args += "-c";
    args += "staff";
#endif

    if ((m_Scheduler != SchedNormal) || (m_Priority > 50))
        args += "root";
    else
        args += m_User;
    
    if (superUserCommand == "su") {
        args += "-c";
    }
    args += QCString(__KDE_BINDIR) + "/kdesu_stub";
#ifndef Q_OS_DARWIN
    args += "-";
#endif

    ///    QCString command = __PATH_SU;
    ///    if (::access(__PATH_SU, X_OK) != 0)
    QCString command;
    if (superUserCommand == "sudo") {
        command = __PATH_SUDO;
    } else {
        command = __PATH_SU;
    }
 
    if (::access(command, X_OK) != 0)
    {
      ///        command = QFile::encodeName(KGlobal::dirs()->findExe("su"));
        command = QFile::encodeName( KGlobal::dirs()->findExe(superUserCommand.ascii()) );
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
            kdError(900) << k_lineinfo << "Conversation with " << superUserCommand << " failed\n";
        return ret;
    }
    if (check == NeedPassword)
    {
        if (ret == killme)
        {
	  ///            if (kill(m_Pid, SIGKILL) < 0)
	  ///            {
	  ///                ret=error;
	  ///            }
            if ( superUserCommand == "sudo" ) {
 	        // sudo can not be killed, just return
 	        return ret;
 	    }
 	    if (kill(m_Pid, SIGKILL) < 0) {
 	        kdDebug() << k_funcinfo << "kill < 0" << endl;
 		//FIXME SIGKILL doesn't work for sudo,
 		//why is this different from su?
 		ret=error;
 	    }
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
        const uint plen = strlen(password);
        for (unsigned i=0; i < plen; i++)
            ptr[i] = '\000';
    }

    if (ret == notauthorized)
    {
        kill(m_Pid, SIGKILL);
        if (superUserCommand != "sudo") {
            waitForChild();
        }
        return SuIncorrectPassword;
    }

    int iret = ConverseStub(check);
    if (iret < 0)
    {
        if (!check)
            kdError(900) << k_lineinfo << "Converstation with kdesu_stub failed\n";
        return iret;
    }
    else if (iret == 1)
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
            //////////////////////////////////////////////////////////////////////////
            case WaitForPrompt:
            {
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
                    if (more.isEmpty())
                        break;
    
                    line = more;
                    kdDebug(900) << k_lineinfo << "Read line <" << more << ">" << endl;
                }
    
                // Match "Password: " with the regex ^[^:]+:[\w]*$.
                const uint len = line.length();
                for (i=0,j=0,colon=0; i<len; i++) 
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
                        kdError(900) << superUserCommand << " has exited while waiting for pwd." << endl;
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
            }
            //////////////////////////////////////////////////////////////////////////
            case CheckStar:
            {
                QCString s = line.stripWhiteSpace();
                if (s.isEmpty()) 
                {
                    state=HandleStub;
                    break;
                }
                const uint len = line.length();
                for (i=0; i< len; i++)
                    {
                if (s[i] != '*')
                    return error;
                }
                state=HandleStub;
                break;
            }
            //////////////////////////////////////////////////////////////////////////
            case HandleStub:
                // Read till we get "kdesu_stub"
                if (line == "kdesu_stub")
                {
                    unreadLine(line);
                    return ok;
 	        } else if (superUserCommand == "sudo") {
 	            // sudo gives a "sorry" line so reaches here
 	            // with the wrong password
 	            return notauthorized;
                }
                break;
            //////////////////////////////////////////////////////////////////////////
        } // end switch
    } // end while (true)
    return ok;
}

void SuProcess::virtual_hook( int id, void* data )
{ StubProcess::virtual_hook( id, data ); }


