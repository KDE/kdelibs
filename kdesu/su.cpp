/* vi: ts=8 sts=4 sw=4
*
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

#include <config.h>
#include <config-prefix.h> // for LIBEXEC_INSTALL_DIR

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

#include <QtCore/QFile>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kuser.h>

int kdesuDebugArea()
{
    static int s_area = KDebug::registerArea("kdesu (kdelibs)");
    return s_area;
}

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
    QString m_superUserCommand;
};

SuProcess::SuProcess(const QByteArray &user, const QByteArray &command)
    : d( new SuProcessPrivate )
{
    m_User = user;
    m_Command = command;

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group(config, "super-user-command");
    d->m_superUserCommand = group.readEntry("super-user-command", DEFAULT_SUPER_USER_COMMAND);

    if ( d->m_superUserCommand != "sudo" && d->m_superUserCommand != "su" ) {
      kWarning() << "unknown super user command.";
      d->m_superUserCommand = DEFAULT_SUPER_USER_COMMAND;
    }
}


SuProcess::~SuProcess()
{
    delete d;
}

QString SuProcess::superUserCommand()
{
    return d->m_superUserCommand;
}

bool SuProcess::useUsersOwnPassword()
{
    if (superUserCommand() == "sudo" && m_User == "root") {
        return true;
    }

    KUser user;
    return user.loginName() == m_User;
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
        d->m_superUserCommand = "su";
    }

    QList<QByteArray> args;
    if (d->m_superUserCommand == "sudo") {
        args += "-u";
    }

    if ((m_Scheduler != SchedNormal) || (m_Priority > 50))
        args += "root";
    else
        args += m_User;

    if (d->m_superUserCommand == "su") {
        args += "-c";
    }
    args += QByteArray(LIBEXEC_INSTALL_DIR) + "/kdesu_stub";
    args += "-"; // krazy:exclude=doublequote_chars (QList, not QString)

    QByteArray command;
    if (d->m_superUserCommand == "sudo") {
        command = __PATH_SUDO;
    } else {
        command = __PATH_SU;
    }

    if (::access(command, X_OK) != 0)
    {
        command = QFile::encodeName( KGlobal::dirs()->findExe(d->m_superUserCommand.toLatin1()) );
        if (command.isEmpty())
            return check ? SuNotFound : -1;
    }

    // kDebug(kdesuDebugArea()) << k_lineinfo << "Call StubProcess::exec()";
    if (StubProcess::exec(command, args) < 0)
    {
        return check ? SuNotFound : -1;
    }
    // kDebug(kdesuDebugArea()) << k_lineinfo << "Done StubProcess::exec()";

    SuErrors ret = (SuErrors) ConverseSU(password);
    // kDebug(kdesuDebugArea()) << k_lineinfo << "Conversation returned" << ret;

    if (ret == error)
    {
        if (!check)
            kError(kdesuDebugArea()) << k_lineinfo << "Conversation with su failed.";
        return ret;
    }
    if (check == NeedPassword)
    {
        if (ret == killme)
        {
            if ( d->m_superUserCommand == "sudo" ) {
 	        // sudo can not be killed, just return
 	        return ret;
 	    }
 	    if (kill(m_Pid, SIGKILL) < 0) {
 	        kDebug() << "kill < 0";
 		//FIXME SIGKILL doesn't work for sudo,
 		//why is this different from su?
 		//A: because sudo runs as root. Perhaps we could write a Ctrl+C to its stdin, instead?
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
        memset(const_cast<char *>(password), 0, qstrlen(password));

    if (ret != ok)
    {
        kill(m_Pid, SIGKILL);
        if (d->m_superUserCommand != "sudo") {
            waitForChild();
        }
        return SuIncorrectPassword;
    }

    int iret = ConverseStub(check);
    if (iret < 0)
    {
        if (!check)
            kError(kdesuDebugArea()) << k_lineinfo << "Conversation with kdesu_stub failed.";
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
    // kDebug(kdesuDebugArea()) << k_lineinfo << "ConverseSU starting.";

    QByteArray line;
    while (true)
    {
        line = readLine();
        if (line.isNull())
            return ( state == HandleStub ? notauthorized : error);
        kDebug(kdesuDebugArea()) << k_lineinfo << "Read line" << line;

        if (line == "kdesu_stub")
        {
            unreadLine(line);
            return ok;
        }

        switch (state)
        {
            //////////////////////////////////////////////////////////////////////////
            case WaitForPrompt:
            {
                if (waitMS(fd(),100)>0)
                {
                    // There is more output available, so this line
                    // couldn't have been a password prompt (the definition
                    // of prompt being that  there's a line of output followed
                    // by a colon, and then the process waits).
                    continue;
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
                        kError(kdesuDebugArea()) << "su has exited while waiting for pwd." << endl;
                        return error;
                    }
                    if ((WaitSlave() == 0) && checkPid(m_Pid))
                    {
                        write(fd(), password, strlen(password));
                        write(fd(), "\n", 1);
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
                QByteArray s = line.trimmed();
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
                if (!line.isEmpty ()) {
                    // if we read anything but the stub response, we assume it's
                    // sorry, wrong password.
                    // FIXME in reality, this may be lack of permission to
                    // execute kdesu_stub via sudo.
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

}
