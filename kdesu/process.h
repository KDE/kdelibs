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
 */

#ifndef __Process_h_Included__
#define __Process_h_Included__

#include <qcstring.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>

class PTY;
typedef QValueList<QCString> QCStringList;

/**
 * Synchronous communication with tty programs.
 *
 * PtyProcess provides synchronous communication with tty based programs. 
 * The communications channel used is a pseudo tty (as opposed to a pipe) 
 * This means that programs which require a terminal will work.
 */

class PtyProcess
{
public:
    PtyProcess();
    virtual ~PtyProcess();

    /**
     * Forks off and execute a command. The command's standard in and output 
     * are connected to the pseudo tty. They are accessible with @ref #readLine 
     * and @ref #writeLine.
     * @param command The command to execute.
     * @param args The arguments to the command.
     */
    int exec(const QCString &command, const QCStringList &args);

    /**
     * Reads a line from the program's standard out. Depending on the @em block 
     * parameter, this call blocks until a single, full line is read. 
     * @param block Block until a full line is read?
     * @return The output string.
     */
    QCString readLine(bool block=true);

    /**
     * Writes a line of text to the program's standard in.
     * @param line The text to write.
     * @param addNewline Adds a '\n' to the line.
     */
    void writeLine(const QCString &line, bool addNewline=true);

    /**
     * Puts back a line of input.
     * @param line The line to put back.
     * @param addNewline Adds a '\n' to the line.
     */
    void unreadLine(const QCString &line, bool addNewline=true);

    /**
     * Sets the exit string. If a line of program output matches this,
     * @ref #waitForChild() will terminate the program and return.
     */
    void setExitString(const QCString &exit) { m_Exit = exit; }

    /**
     * Waits for the child to exit. See also @ref #setExitString.
     */
    int waitForChild();

    /**
     * Waits until the pty has cleared the ECHO flag. This is usefull 
     * when programs write a password prompt before they disable ECHO.
     * Disabling it might flush any input that was written.
     */
    int WaitSlave();

    /**
     * Enables/disables local echo on the pseudo tty.
     */
    int enableLocalEcho(bool enable=true);

    /**
     * Enables/disables terminal output. Relevant only to some subclasses.
     */
    void setTerminal(bool terminal) { m_bTerminal = terminal; }

    /**
     * Overwrites the password as soon as it is used. Relevant only to
     * some subclasses.
     */
    void setErase(bool erase) { m_bErase = erase; }

    /**
     * Set additinal environment variables.
     */
    void setEnvironment( const QCStringList &env );

    /**
     * Returns the filedescriptor of the process.
     */
    int fd() {return m_Fd;};

    /**
     * Returns the pid of the process.
     */
    int pid() {return m_Pid;};

protected:
    const QCStringList& environment() const;

    bool m_bErase, m_bTerminal;
    int m_Pid, m_Fd;
    QCString m_Command, m_Exit;

private:
    int init();
    int SetupTTY(int fd);

    PTY *m_pPTY;
    QCString m_Inbuf, m_TTY;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class PtyProcessPrivate;
    PtyProcessPrivate *d;
};

#endif
