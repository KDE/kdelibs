/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * This file contains code from TEShell.C of the KDE konsole.
 * Copyright (c) 1997,1998 by Lars Doelle <lars.doelle@on-line.de>
 *
 * This is free software; you can use this library under the GNU Library
 * General Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 *
 * process.cpp: Functionality to build a front end to password asking
 *  terminal programs.
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ioctl.h>

#if defined(__SVR4) && defined(sun)
#include <stropts.h>
#include <sys/stream.h>
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>                // Needed on some systems.
#endif

#include <qglobal.h>
#include <qfile.h>

#include <kdebug.h>
#include <kstandarddirs.h>

#include "process.h"
#include "kdesu_pty.h"
#include "kcookie.h"

int PtyProcess::waitMS(int fd,int ms)
{
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 1000*ms;

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(fd,&fds);
	return select(fd+1, &fds, 0L, 0L, &tv);
}

/*
** Basic check for the existence of @p pid.
** Returns true iff @p pid is an extant process.
*/
bool PtyProcess::checkPid(pid_t pid)
{
	return kill(pid,0) == 0;
}

/*
** Check process exit status for process @p pid.
** On error (no child, no exit), return Error (-1).
** If child @p pid has exited, return its exit status,
** (which may be zero).
** If child @p has not exited, return NotExited (-2).
*/

int PtyProcess::checkPidExited(pid_t pid)
{
	int state, ret;
	ret = waitpid(pid, &state, WNOHANG);

	if (ret < 0)
	{
		kdError(900) << k_lineinfo << "waitpid(): " << perror << "\n";
		return Error;
	}
	if (ret == pid)
	{
		if (WIFEXITED(state))
			return WEXITSTATUS(state);
		return Killed;
	}

	return NotExited;
}


class PtyProcess::PtyProcessPrivate
{
public:
    QList<QByteArray> env;
};


PtyProcess::PtyProcess()
    :d(new PtyProcessPrivate)
{
    m_bTerminal = false;
    m_bErase = false;
    m_pPTY = 0L;
}


int PtyProcess::init()
{
    delete m_pPTY;
    m_pPTY = new PTY();
    m_Fd = m_pPTY->getpt();
    if (m_Fd < 0)
        return -1;
    if ((m_pPTY->grantpt() < 0) || (m_pPTY->unlockpt() < 0))
    {
        kdError(900) << k_lineinfo << "Master setup failed.\n";
        m_Fd = -1;
        return -1;
    }
    m_TTY = m_pPTY->ptsname();
    m_Inbuf.resize(0);
    return 0;
}


PtyProcess::~PtyProcess()
{
    delete m_pPTY;
    delete d;
}

/** Set additinal environment variables. */
void PtyProcess::setEnvironment( const QList<QByteArray> &env )
{
    d->env = env;
}

QList<QByteArray> PtyProcess::environment() const
{
    return d->env;
}

/*
 * Read one line of input. The terminal is in canonical mode, so you always
 * read a line at at time, but it's possible to receive multiple lines in
 * one time.
 */

QByteArray PtyProcess::readLine(bool block)
{
    int pos;
    QByteArray ret;

    if (!m_Inbuf.isEmpty())
    {
        pos = m_Inbuf.indexOf('\n');
        if (pos == -1)
        {
            ret = m_Inbuf;
            m_Inbuf.resize(0);
        } else
        {
            ret = m_Inbuf.left(pos);
            m_Inbuf = m_Inbuf.mid(pos+1);
        }
        return ret;
    }

    int flags = fcntl(m_Fd, F_GETFL);
    if (flags < 0)
    {
        kdError(900) << k_lineinfo << "fcntl(F_GETFL): " << perror << "\n";
        return ret;
    }
    int oflags = flags;
    if (block)
        flags &= ~O_NONBLOCK;
    else
        flags |= O_NONBLOCK;

    if ((flags != oflags) && (fcntl(m_Fd, F_SETFL, flags) < 0))
    {
       // We get an error here when the child process has closed 
       // the file descriptor already.
       return ret;
    }

    int nbytes;
    char buf[256];
    while (1)
    {
        nbytes = read(m_Fd, buf, 255);
        if (nbytes == -1)
        {
            if (errno == EINTR)
                continue;
            else break;
        }
        if (nbytes == 0)
            break;        // eof

        buf[nbytes] = '\000';
        m_Inbuf += buf;

        pos = m_Inbuf.indexOf('\n');
        if (pos == -1)
        {
            ret = m_Inbuf;
            m_Inbuf.resize(0);
        } else
        {
            ret = m_Inbuf.left(pos);
            m_Inbuf = m_Inbuf.mid(pos+1);
        }
        break;
    }

    return ret;
}


void PtyProcess::writeLine(const QByteArray &line, bool addnl)
{
    if (!line.isEmpty())
        write(m_Fd, line, line.length());
    if (addnl)
        write(m_Fd, "\n", 1);
}


void PtyProcess::unreadLine(const QByteArray &line, bool addnl)
{
    QByteArray tmp = line;
    if (addnl)
        tmp += '\n';
    if (!tmp.isEmpty())
        m_Inbuf.prepend(tmp);
}

/*
 * Fork and execute the command. This returns in the parent.
 */

int PtyProcess::exec(const QByteArray &command, const QList<QByteArray> &args)
{
    kdDebug(900) << k_lineinfo << "Running `" << command << "'\n";
    int i;

    if (init() < 0)
        return -1;

    // Open the pty slave before forking. See SetupTTY()
    int slave = open(m_TTY, O_RDWR);
    if (slave < 0)
    {
        kdError(900) << k_lineinfo << "Could not open slave pty.\n";
        return -1;
    }

    if ((m_Pid = fork()) == -1)
    {
        kdError(900) << k_lineinfo << "fork(): " << perror << "\n";
        return -1;
    }

    // Parent
    if (m_Pid)
    {
        close(slave);
        return 0;
    }

    // Child
    if (SetupTTY(slave) < 0)
        _exit(1);

    for (i = 0; i < d->env.count(); ++i)
    {
        putenv(const_cast<char *>(d->env.at(i).constData()));
    }
    unsetenv("KDE_FULL_SESSION");

    // From now on, terminal output goes through the tty.

    QByteArray path;
    if (command.contains('/'))
        path = command;
    else
    {
        QString file = KStandardDirs::findExe(command);
        if (file.isEmpty())
        {
            kdError(900) << k_lineinfo << command << " not found\n";
            _exit(1);
        }
        path = QFile::encodeName(file);
    }

    const char **argp = (const char **)malloc((args.count()+2)*sizeof(char *));

    i = 0;
    argp[i++] = path;
    for (QList<QByteArray>::ConstIterator it=args.begin(); it!=args.end(); ++it)
        argp[i++] = *it;

    argp[i + 2] = 0;

    execv(path, const_cast<char **>(argp));
    kdError(900) << k_lineinfo << "execv(\"" << path << "\"): " << perror << "\n";
    _exit(1);
    return -1; // Shut up compiler. Never reached.
}


/*
 * Wait until the terminal is set into no echo mode. At least one su
 * (RH6 w/ Linux-PAM patches) sets noecho mode AFTER writing the Password:
 * prompt, using TCSAFLUSH. This flushes the terminal I/O queues, possibly
 * taking the password  with it. So we wait until no echo mode is set
 * before writing the password.
 * Note that this is done on the slave fd. While Linux allows tcgetattr() on
 * the master side, Solaris doesn't.
 */

int PtyProcess::WaitSlave()
{
    int slave = open(m_TTY, O_RDWR);
    if (slave < 0)
    {
        kdError(900) << k_lineinfo << "Could not open slave tty.\n";
        return -1;
    }

    kdDebug(900) << k_lineinfo << "Child pid " << m_Pid << endl;

    struct termios tio;
    while (1)
    {
	if (!checkPid(m_Pid))
	{
		close(slave);
		return -1;
	}
        if (tcgetattr(slave, &tio) < 0)
        {
            kdError(900) << k_lineinfo << "tcgetattr(): " << perror << "\n";
            close(slave);
            return -1;
        }
        if (tio.c_lflag & ECHO)
        {
            kdDebug(900) << k_lineinfo << "Echo mode still on.\n";
	    waitMS(slave,100);
            continue;
        }
        break;
    }
    close(slave);
    return 0;
}


int PtyProcess::enableLocalEcho(bool enable)
{
    int slave = open(m_TTY, O_RDWR);
    if (slave < 0)
    {
        kdError(900) << k_lineinfo << "Could not open slave tty.\n";
        return -1;
    }
    struct termios tio;
    if (tcgetattr(slave, &tio) < 0)
    {
        kdError(900) << k_lineinfo << "tcgetattr(): " << perror << "\n";
        close(slave); return -1;
    }
    if (enable)
        tio.c_lflag |= ECHO;
    else
        tio.c_lflag &= ~ECHO;
    if (tcsetattr(slave, TCSANOW, &tio) < 0)
    {
        kdError(900) << k_lineinfo << "tcsetattr(): " << perror << "\n";
        close(slave); return -1;
    }
    close(slave);
    return 0;
}


/*
 * Copy output to stdout until the child process exists, or a line of output
 * matches `m_Exit'.
 * We have to use waitpid() to test for exit. Merely waiting for EOF on the
 * pty does not work, because the target process may have children still
 * attached to the terminal.
 */

int PtyProcess::waitForChild()
{
    int retval = 1;

    fd_set fds;
    FD_ZERO(&fds);

    while (1)
    {
        FD_SET(m_Fd, &fds);
        int ret = select(m_Fd+1, &fds, 0L, 0L, 0L);
        if (ret == -1)
        {
            if (errno != EINTR)
            {
                kdError(900) << k_lineinfo << "select(): " << perror << "\n";
                return -1;
            }
            ret = 0;
        }

        if (ret)
        {
            QByteArray line = readLine(false);
            while (!line.isNull())
            {
                if (!m_Exit.isEmpty() && !qstrnicmp(line.constData(), m_Exit, m_Exit.length()))
                    kill(m_Pid, SIGTERM);
                if (m_bTerminal)
                {
                    fputs(line, stdout);
                    fputc('\n', stdout);
                }
                line = readLine(false);
            }
        }

	ret = checkPidExited(m_Pid);
	if (ret == Error)
	{
		if (errno == ECHILD) retval = 0;
		else retval = 1;
		break;
	}
	else if (ret == Killed)
	{
		retval = 0;
		break;
	}
	else if (ret == NotExited)
	{
		// keep checking
	}
	else
	{
		retval = ret;
		break;
	}
    }
    return retval;
}

/*
 * SetupTTY: Creates a new session. The filedescriptor "fd" should be
 * connected to the tty. It is closed after the tty is reopened to make it
 * our controlling terminal. This way the tty is always opened at least once
 * so we'll never get EIO when reading from it.
 */

int PtyProcess::SetupTTY(int fd)
{
    // Reset signal handlers
    for (int sig = 1; sig < NSIG; sig++)
        signal(sig, SIG_DFL);
    signal(SIGHUP, SIG_IGN);

    // Close all file handles
    struct rlimit rlp;
    getrlimit(RLIMIT_NOFILE, &rlp);
    for (int i = 0; i < (int)rlp.rlim_cur; i++)
        if (i != fd) close(i);

    // Create a new session.
    setsid();

    // Open slave. This will make it our controlling terminal
    int slave = open(m_TTY, O_RDWR);
    if (slave < 0)
    {
        kdError(900) << k_lineinfo << "Could not open slave side: " << perror << "\n";
        return -1;
    }
    close(fd);

#if defined(__SVR4) && defined(sun)

    // Solaris STREAMS environment.
    // Push these modules to make the stream look like a terminal.
    ioctl(slave, I_PUSH, "ptem");
    ioctl(slave, I_PUSH, "ldterm");

#endif

#ifdef TIOCSCTTY
    ioctl(slave, TIOCSCTTY, NULL);
#endif

    // Connect stdin, stdout and stderr
    dup2(slave, 0); dup2(slave, 1); dup2(slave, 2);
    if (slave > 2)
        close(slave);

    // Disable OPOST processing. Otherwise, '\n' are (on Linux at least)
    // translated to '\r\n'.
    struct termios tio;
    if (tcgetattr(0, &tio) < 0)
    {
        kdError(900) << k_lineinfo << "tcgetattr(): " << perror << "\n";
        return -1;
    }
    tio.c_oflag &= ~OPOST;
    if (tcsetattr(0, TCSANOW, &tio) < 0)
    {
        kdError(900) << k_lineinfo << "tcsetattr(): " << perror << "\n";
        return -1;
    }

    return 0;
}

void PtyProcess::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }
