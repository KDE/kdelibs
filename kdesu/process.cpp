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

#include "process.h"
#include "kcookie.h"

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <termios.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>                // Needed on some systems.
#endif

#include <QtCore/QBool>
#include <QtCore/QFile>

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kde_file.h>

extern int kdesuDebugArea();

namespace KDESu {

using namespace KDESuPrivate;

/*
** Wait for @p ms miliseconds
** @param fd file descriptor
** @param ms time to wait in miliseconds
** @return
*/
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

// XXX this function is nonsense:
// - for our child, we could use waitpid().
// - the configurability at this place it *complete* braindamage
/*
** Basic check for the existence of @p pid.
** Returns true iff @p pid is an extant process.
*/
bool PtyProcess::checkPid(pid_t pid)
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup cg(config, "super-user-command");
    QString superUserCommand = cg.readEntry("super-user-command", "sudo");
    //sudo does not accept signals from user so we except it
    if (superUserCommand == "sudo") {
        return true;
    } else {
        return kill(pid, 0) == 0;
    }
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
		kError(kdesuDebugArea()) << k_lineinfo << "waitpid():" << perror;
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
    PtyProcessPrivate() : m_pPTY(0L) {}
    ~PtyProcessPrivate()
    {
        delete m_pPTY;
    }
    QList<QByteArray> env;
    KPty *m_pPTY;
    QByteArray m_Inbuf;
};


PtyProcess::PtyProcess()
    :d(new PtyProcessPrivate)
{
    m_bTerminal = false;
    m_bErase = false;
}


int PtyProcess::init()
{
    delete d->m_pPTY;
    d->m_pPTY = new KPty();
    if (!d->m_pPTY->open())
    {
        kError(kdesuDebugArea()) << k_lineinfo << "Failed to open PTY.";
        return -1;
    }
    d->m_Inbuf.resize(0);
    return 0;
}


PtyProcess::~PtyProcess()
{
    delete d;
}

/** Set additional environment variables. */
void PtyProcess::setEnvironment( const QList<QByteArray> &env )
{
    d->env = env;
}

int PtyProcess::fd() const
{
    return d->m_pPTY ? d->m_pPTY->masterFd() : -1;
}

int PtyProcess::pid() const
{
    return m_Pid;
}

/** Returns the additional environment variables set by setEnvironment() */
QList<QByteArray> PtyProcess::environment() const
{
    return d->env;
}


QByteArray PtyProcess::readAll(bool block)
{
    QByteArray ret;
    if (!d->m_Inbuf.isEmpty())
    {
        // if there is still something in the buffer, we need not block.
        // we should still try to read any further output, from the fd, though.
        block = false;
        ret = d->m_Inbuf;
        d->m_Inbuf.resize(0);
    }

    int flags = fcntl(fd(), F_GETFL);
    if (flags < 0)
    {
        kError(kdesuDebugArea()) << k_lineinfo << "fcntl(F_GETFL):" << perror;
        return ret;
    }
    int oflags = flags;
    if (block)
        flags &= ~O_NONBLOCK;
    else
        flags |= O_NONBLOCK;

    if ((flags != oflags) && (fcntl(fd(), F_SETFL, flags) < 0))
    {
       // We get an error here when the child process has closed
       // the file descriptor already.
       return ret;
    }

    int nbytes;
    char buf[256];
    while (1)
    {
        nbytes = read(fd(), buf, 255);
        if (nbytes == -1)
        {
            if (errno == EINTR)
                continue;
            else break;
        }
        if (nbytes == 0)
            break;        // nothing available / eof

        buf[nbytes] = '\000';
        ret += buf;
        break;
    }

    return ret;
}


QByteArray PtyProcess::readLine(bool block)
{
    d->m_Inbuf = readAll(block);

    int pos;
    QByteArray ret;
    if (!d->m_Inbuf.isEmpty())
    {
        pos = d->m_Inbuf.indexOf('\n');
        if (pos == -1)
        {
            // NOTE: this means we return something even if there in no full line!
            ret = d->m_Inbuf;
            d->m_Inbuf.resize(0);
        } else
        {
            ret = d->m_Inbuf.left(pos);
            d->m_Inbuf = d->m_Inbuf.mid(pos+1);
        }
    }

    return ret;
}


void PtyProcess::writeLine(const QByteArray &line, bool addnl)
{
    if (!line.isEmpty())
        write(fd(), line, line.length());
    if (addnl)
        write(fd(), "\n", 1);
}


void PtyProcess::unreadLine(const QByteArray &line, bool addnl)
{
    QByteArray tmp = line;
    if (addnl)
        tmp += '\n';
    if (!tmp.isEmpty())
        d->m_Inbuf.prepend(tmp);
}

void PtyProcess::setExitString(const QByteArray &exit)
{
    m_Exit = exit;
}

/*
 * Fork and execute the command. This returns in the parent.
 */

int PtyProcess::exec(const QByteArray &command, const QList<QByteArray> &args)
{
    kDebug(kdesuDebugArea()) << k_lineinfo << "Running" << command;
    int i;

    if (init() < 0)
        return -1;

    if ((m_Pid = fork()) == -1)
    {
        kError(kdesuDebugArea()) << k_lineinfo << "fork():" << perror;
        return -1;
    }

    // Parent
    if (m_Pid)
    {
        d->m_pPTY->closeSlave();
        return 0;
    }

    // Child
    if (setupTTY() < 0)
        _exit(1);

    for (i = 0; i < d->env.count(); ++i)
    {
        putenv(const_cast<char *>(d->env.at(i).constData()));
    }
    unsetenv("KDE_FULL_SESSION");
    // for : Qt: Session management error
    unsetenv("SESSION_MANAGER");
    // QMutex::lock , deadlocks without that.
    // <thiago> you cannot connect to the user's session bus from another UID
    unsetenv("DBUS_SESSION_BUS_ADDRESS");

    // set temporarily LC_ALL to C, for su (to be able to parse "Password:")
    const QByteArray old_lc_all = qgetenv( "LC_ALL" );
    if( !old_lc_all.isEmpty() )
        qputenv( "KDESU_LC_ALL", old_lc_all );
    else
        unsetenv( "KDESU_LC_ALL" );
    qputenv("LC_ALL", "C");

    // From now on, terminal output goes through the tty.

    QByteArray path;
    if (command.contains('/'))
        path = command;
    else
    {
        QString file = KStandardDirs::findExe(command);
        if (file.isEmpty())
        {
            kError(kdesuDebugArea()) << k_lineinfo << command << "not found.";
            _exit(1);
        }
        path = QFile::encodeName(file);
    }

    const char **argp = (const char **)malloc((args.count()+2)*sizeof(char *));

    i = 0;
    argp[i++] = path;
    for (QList<QByteArray>::ConstIterator it=args.begin(); it!=args.end(); ++it, ++i)
        argp[i] = *it;

    argp[i] = NULL;

    execv(path, const_cast<char **>(argp));
    kError(kdesuDebugArea()) << k_lineinfo << "execv(" << path << "):" << perror;
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
    kDebug(kdesuDebugArea()) << k_lineinfo << "Child pid" << m_Pid;

    struct termios tio;
    while (1)
    {
        if (!checkPid(m_Pid))
        {
            return -1;
        }
        if (!d->m_pPTY->tcGetAttr(&tio))
        {
            kError(kdesuDebugArea()) << k_lineinfo << "tcgetattr():" << perror;
            return -1;
        }
        if (tio.c_lflag & ECHO)
        {
            kDebug(kdesuDebugArea()) << k_lineinfo << "Echo mode still on.";
            usleep(10000);
            continue;
        }
        break;
    }
    return 0;
}


int PtyProcess::enableLocalEcho(bool enable)
{
    return d->m_pPTY->setEcho(enable) ? 0 : -1;
}


void PtyProcess::setTerminal(bool terminal)
{
    m_bTerminal = terminal;
}

void PtyProcess::setErase(bool erase)
{
    m_bErase = erase;
}

/*
 * Copy output to stdout until the child process exits, or a line of output
 * matches `m_Exit'.
 * We have to use waitpid() to test for exit. Merely waiting for EOF on the
 * pty does not work, because the target process may have children still
 * attached to the terminal.
 */

int PtyProcess::waitForChild()
{
    fd_set fds;
    FD_ZERO(&fds);
    QByteArray remainder;

    while (1)
    {
        FD_SET(fd(), &fds);

        // specify timeout to make sure select() does not block, even if the
        // process is dead / non-responsive. It does not matter if we abort too
        // early. In that case 0 is returned, and we'll try again in the next
        // iteration. (As long as we don't consitently time out in each iteration)
        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        int ret = select(fd()+1, &fds, 0L, 0L, &timeout);
        if (ret == -1)
        {
            if (errno != EINTR)
            {
                kError(kdesuDebugArea()) << k_lineinfo << "select():" << perror;
                return -1;
            }
            ret = 0;
        }

        if (ret)
        {
            forever {
                QByteArray output = readAll(false);
                if (output.isEmpty())
                    break;
                if (m_bTerminal)
                {
                    fwrite(output.constData(), output.size(), 1, stdout);
                    fflush(stdout);
                }
                if (!m_Exit.isEmpty())
                {
                    // match exit string only at line starts
                    remainder += output;
                    while (remainder.length() >= m_Exit.length()) {
                        if (remainder.startsWith(m_Exit)) {
                            kill(m_Pid, SIGTERM);
                            remainder.remove(0, m_Exit.length());
                        }
                        int off = remainder.indexOf('\n');
                        if (off < 0)
                            break;
                        remainder.remove(0, off + 1);
                    }
                }
            }
        }

        ret = checkPidExited(m_Pid);
        if (ret == Error)
        {
            if (errno == ECHILD) return 0;
            else return 1;
        }
        else if (ret == Killed)
        {
            return 0;
        }
        else if (ret == NotExited)
        {
            // keep checking
        }
        else
        {
            return ret;
        }
    }
}

/*
 * SetupTTY: Creates a new session. The filedescriptor "fd" should be
 * connected to the tty. It is closed after the tty is reopened to make it
 * our controlling terminal. This way the tty is always opened at least once
 * so we'll never get EIO when reading from it.
 */

int PtyProcess::setupTTY()
{
    // Reset signal handlers
    for (int sig = 1; sig < NSIG; sig++)
        KDE_signal(sig, SIG_DFL);
    KDE_signal(SIGHUP, SIG_IGN);

    d->m_pPTY->setCTty();

    // Connect stdin, stdout and stderr
    int slave = d->m_pPTY->slaveFd();
    dup2(slave, 0); dup2(slave, 1); dup2(slave, 2);

    // Close all file handles
    // XXX this caused problems in KProcess - not sure why anymore. -- ???
    // Because it will close the start notification pipe. -- ossi
    struct rlimit rlp;
    getrlimit(RLIMIT_NOFILE, &rlp);
    for (int i = 3; i < (int)rlp.rlim_cur; i++)
        close(i);

    // Disable OPOST processing. Otherwise, '\n' are (on Linux at least)
    // translated to '\r\n'.
    struct ::termios tio;
    if (tcgetattr(0, &tio) < 0)
    {
        kError(kdesuDebugArea()) << k_lineinfo << "tcgetattr():" << perror;
        return -1;
    }
    tio.c_oflag &= ~OPOST;
    if (tcsetattr(0, TCSANOW, &tio) < 0)
    {
        kError(kdesuDebugArea()) << k_lineinfo << "tcsetattr():" << perror;
        return -1;
    }

    return 0;
}

void PtyProcess::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

}
