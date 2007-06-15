/*
    This file is part of the KDE libraries

    Copyright (C) 2007 Oswald Buddenhagen <ossi@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kprocess_p.h"

#ifdef Q_OS_WIN
# include <windows.h>
#else
# include <unistd.h>
# include <errno.h>
#endif

#ifndef Q_OS_WIN
# define STD_OUTPUT_HANDLE 1
# define STD_ERROR_HANDLE 2
#endif

void KProcessPrivate::writeAll(const QByteArray &buf, int fd)
{
#ifdef Q_OS_WIN
    HANDLE h = GetStdHandle(fd);
    if (h) {
        DWORD wr;
        WriteFile(h, buf.data(), buf.size(), &wr, 0);
    }
#else
    int off = 0;
    do {
        int ret = ::write(fd, buf.data() + off, buf.size() - off);
        if (ret < 0) {
            if (errno != EINTR)
                return;
        } else {
            off += ret;
        }
    } while (off < buf.size());
#endif
}

void KProcessPrivate::forwardStd(KProcess::ProcessChannel good, KProcess::ProcessChannel bad, int fd)
{
    Q_Q(KProcess);

    if (q->readChannel() == good)
        writeAll(q->readAll(), fd);
    else {
        // workaround for QProcess dropping the read buffer on channel switch
        int ba = q->bytesAvailable();
        QByteArray buf = q->read(ba);

        q->setReadChannel(good);
        writeAll(q->readAll(), fd);
        q->setReadChannel(bad);

        // how "efficient" ...
        for (int i = ba; --i >= 0;)
            q->ungetChar(buf[i]);
    }
}

void KProcessPrivate::_k_forwardStdout()
{
    forwardStd(KProcess::StandardOutput, KProcess::StandardError, STD_OUTPUT_HANDLE);
}

void KProcessPrivate::_k_forwardStderr()
{
    forwardStd(KProcess::StandardError, KProcess::StandardOutput, STD_ERROR_HANDLE);
}

/////////////////////////////
// public member functions //
/////////////////////////////

KProcess::KProcess(QObject *parent) :
    QProcess(parent),
    d_ptr(new KProcessPrivate)
{
    d_ptr->q_ptr = this;
    setOutputChannelMode(ForwardedChannels);
}

KProcess::KProcess(KProcessPrivate *d, QObject *parent) :
    QProcess(parent),
    d_ptr(d)
{
    d_ptr->q_ptr = this;
    setOutputChannelMode(ForwardedChannels);
}

KProcess::~KProcess()
{
    delete d_ptr;
}

void KProcess::setOutputChannelMode(OutputChannelMode mode)
{
    Q_D(KProcess);

    d->outputChannelMode = mode;
    disconnect(this, SIGNAL(readyReadStandardOutput()));
    disconnect(this, SIGNAL(readyReadStandardError()));
    switch (mode) {
    case OnlyStdoutChannel:
        connect(this, SIGNAL(readyReadStandardError()), SLOT(_k_forwardStderr()));
        break;
    case OnlyStderrChannel:
        connect(this, SIGNAL(readyReadStandardOutput()), SLOT(_k_forwardStdout()));
        break;
    default:
        QProcess::setProcessChannelMode((ProcessChannelMode)mode);
        return;
    }
    QProcess::setProcessChannelMode(QProcess::SeparateChannels);
}

KProcess::OutputChannelMode KProcess::outputChannelMode() const
{
    Q_D(const KProcess);

    return d->outputChannelMode;
}

void KProcess::setNextOpenMode(QIODevice::OpenMode mode)
{
    Q_D(KProcess);

    d->openMode = mode;
}

#define DUMMYENV "_KPROCESS_DUMMY_="

void KProcess::clearEnvironment()
{
    setEnvironment(QStringList() << QString::fromLatin1(DUMMYENV));
}

void KProcess::setEnv(const QString &name, const QString &value, bool overwrite)
{
    QStringList env = environment();
    if (env.isEmpty()) {
        env = systemEnvironment();
        env.removeAll(QString::fromLatin1(DUMMYENV));
    }
    QString fname(name);
    fname.append('=');
    for (QStringList::Iterator it = env.begin(); it != env.end(); ++it)
        if ((*it).startsWith(fname)) {
            if (overwrite) {
                *it = fname.append(value);
                setEnvironment(env);
            }
            return;
        }
    env.append(fname.append(value));
    setEnvironment(env);
}

void KProcess::unsetEnv(const QString &name)
{
    QStringList env = environment();
    if (env.isEmpty()) {
        env = systemEnvironment();
        env.removeAll(QString::fromLatin1(DUMMYENV));
    }
    QString fname(name);
    fname.append('=');
    for (QStringList::Iterator it = env.begin(); it != env.end(); ++it)
        if ((*it).startsWith(fname)) {
            env.erase(it);
            if (env.isEmpty())
                env.append(DUMMYENV);
            setEnvironment(env);
            return;
        }
}

void KProcess::setProgram(const QString &exe, const QStringList &args)
{
    Q_D(KProcess);

    d->prog = exe;
    d->args = args;
}

void KProcess::setProgram(const QStringList &argv)
{
    Q_D(KProcess);

    d->args = argv;
    d->prog = d->args.takeFirst();
}

KProcess &KProcess::operator<<(const QString &arg)
{
    Q_D(KProcess);

    if (d->prog.isEmpty())
        d->prog = arg;
    else
        d->args << arg;
    return *this;
}

KProcess &KProcess::operator<<(const QStringList &args)
{
    Q_D(KProcess);

    if (d->prog.isEmpty())
        setProgram(args);
    else
        d->args << args;
    return *this;
}

void KProcess::setShellCommand(const QString &cmd, const QString &shell)
{
    Q_D(KProcess);

    if (!shell.isEmpty()) {
        d->prog = shell;
    } else {
#ifdef Q_OS_UNIX
        d->prog = QString::fromLatin1(
// #ifdef NON_FREE // ... as they ship non-POSIX /bin/sh
# if !defined(__linux__) && !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__OpenBSD__) && !defined(__DragonFly__) && !defined(__GNU__)
            !access("/usr/xpg4/bin/sh", X_OK) ? // Solaris POSIX ...
                "/usr/xpg4/bin/sh" :
            !access("/bin/ksh", X_OK) ? // ... which links here anyway
                "/bin/ksh" :
            !access("/usr/ucb/sh", X_OK) ? // dunno, maybe superfluous?
                "/usr/ucb/sh" :
# endif
                "/bin/sh"
        );
#else // Q_OS_UNIX
        d->prog = QString::fromLocal8Bit(qgetenv("ComSpec"));
/*
    really needed?
    if(GetFileAttributesW(cmd.utf16()) == INVALID_FILE_ATTRIBUTES)
        return; // mhhh
*/
#endif // Q_OS_UNIX
    }
#ifdef Q_OS_UNIX
    d->args = QStringList() << "-c" << cmd;
#else
    // XXX this has a good chance of being broken, definitely for command.com.
    // split on whitespace and make a list of that, maybe? don't forget
    // about consecutive spaces in this case.
    // see also TrollTechTaskTracker entry 88373.
    d->args = QStringList() << "/c" << cmd;
#endif
}

void KProcess::start()
{
    Q_D(KProcess);

    QProcess::start(d->prog, d->args, d->openMode);
}

int KProcess::execute(int msecs)
{
    start();
    if (!waitForFinished(msecs)) {
        kill();
        waitForFinished(-1);
        return -1;
    }
    return (exitStatus() == QProcess::NormalExit) ? exitCode() : -1;
}

// static
int KProcess::execute(const QString &exe, const QStringList &args, int msecs)
{
    KProcess p;
    p.setProgram(exe, args);
    return p.execute(msecs);
}

// static
int KProcess::execute(const QStringList &argv, int msecs)
{
    KProcess p;
    p.setProgram(argv);
    return p.execute(msecs);
}

int KProcess::pid() const
{
#ifdef Q_OS_UNIX
    return (int) QProcess::pid();
#else
    return (int) QProcess::pid()->dwProcessId;
#endif
}

#include "kprocess.moc"
