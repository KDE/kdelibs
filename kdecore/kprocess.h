/* This file is part of the KDE libraries
    Copyright (C) 1997 Christian Czezakte (e9025461@student.tuwien.ac.at)

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
//
//  KPROCESS -- A class for handling child processes in KDE without
//  having to take care of Un*x specific implementation details
//
//  version 0.3.1, Jan 8th 1998
//
//  (C) Christian Czezatke
//  e9025461@student.tuwien.ac.at
//

#ifndef __kprocess_h__
#define __kprocess_h__

#include <sys/types.h> // for pid_t
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <qvaluelist.h>
#include <qcstring.h>
#include <qobject.h>

class QSocketNotifier;
class KProcessPrivate;

/**
 * Child process invocation, monitoring and control.
 *
 * @sect General usage and features
 *
 *This class allows a KDE application to start child processes without having
 *to worry about UN*X signal handling issues and zombie process reaping.
 *
 *@see KProcIO
 *
 *Basically, this class distinguishes three different ways of running
 *child processes:
 *
 *@li  KProcess::DontCare -- The child process is invoked and both the child
 *process and the parent process continue concurrently.
 *
 *Starting a  DontCare child process means that the application is
 *not interested in any notification to determine whether the
 *child process has already exited or not.
 *
 *@li  KProcess::NotifyOnExit -- The child process is invoked and both the
 *child and the parent process run concurrently.
 *
 *When the child process exits, the KProcess instance
 *corresponding to it emits the Qt signal @ref processExited().
 *
 *Since this signal is @em not emitted from within a UN*X
 *signal handler, arbitrary function calls can be made.
 *
 *Be aware: When the KProcess objects gets destructed, the child
 *process will be killed if it is still running!
 *This means in particular, that you cannot use a KProcess on the stack
 *with KProcess::NotifyOnExit.
 *
 *@li  KProcess::Block -- The child process starts and the parent process
 *is suspended until the child process exits. (@em Really not recommended
 *for programs with a GUI.)
 *
 *KProcess also provides several functions for determining the exit status
 *and the pid of the child process it represents.
 *
 *Furthermore it is possible to supply command-line arguments to the process
 *in a clean fashion (no null -- terminated stringlists and such...)
 *
 *A small usage example:
 *<pre>
 *KProcess *proc = new KProcess;
 *
 **proc << "my_executable";
 **proc << "These" << "are" << "the" << "command" << "line" << "args";
 *QApplication::connect(proc, SIGNAL(processExited(KProcess *)),
 *                      pointer_to_my_object, SLOT(my_objects_slot(KProcess *)));
 *proc->start();
 *</pre>
 *
 *This will start "my_executable" with the commandline arguments "These"...
 *
 *When the child process exits, the respective Qt signal will be emitted.
 *
 *@sect Communication with the child process
 *
 *KProcess supports communication with the child process through
 *stdin/stdout/stderr.
 *
 *The following functions are provided for getting data from the child
 *process or sending data to the child's stdin (For more information,
 *have a look at the documentation of each function):
 *
 *@li bool @ref writeStdin(char  *buffer, int  buflen);
 *@li  -- Transmit data to the child process's stdin.
 *
 *@li bool @ref closeStdin();
 *@li -- Closes the child process's stdin (which causes it to see an  feof(stdin)).
 *Returns false if you try to close stdin for a process that has been started
 *without a communication channel to stdin.
 *
 *@li bool @ref closeStdout();
 *@li -- Closes the child process's stdout.
 *Returns false if you try to close stdout for a process that has been started
 *without a communication channel to stdout.
 *
 *@li bool @ref closeStderr();
 *@li -- Closes the child process's stderr.
 *Returns false if you try to close stderr for a process that has been started
 *without a communication channel to stderr.
 *
 *
 *@sect QT signals:
 *
 *@li void @ref receivedStdout(KProcess  *proc, char  *buffer, int  buflen);
 *@li  void @ref receivedStderr(KProcess  *proc, char  *buffer, int  buflen);
 *@li  -- Indicates that new data has arrived from either the
 *child process's stdout or stderr.
 *
 *@li  void @ref wroteStdin(KProcess  *proc);
 *@li  -- Indicates that all data that has been sent to the child process
 *by a prior call to @ref writeStdin() has actually been transmitted to the
 *client .
 *
 *@author Christian Czezakte e9025461@student.tuwien.ac.at
 *
 *
 **/
class KProcess : public QObject
{
  Q_OBJECT

public:

  /**
   * Modes in which the communication channel can be opened.
   *
   * If communication for more than one channel is required,
   * the values have to be or'ed together, for example to get
   * communication with stdout as well as with stdin, you would
   * specify @p Stdin @p | @p Stdout
   *
   * If @p NoRead is specified in conjunction with @p Stdout,
   * no data is actually read from @p Stdout but only
   * the signal @ref childOutput(int fd) is emitted.
   */
  enum Communication { NoCommunication = 0, Stdin = 1, Stdout = 2, Stderr = 4,
					   AllOutput = 6, All = 7,
					   NoRead };

  /**
   * Run-modes for a child process.
   */
  enum RunMode {
      /**
       * The application does not receive notifications from the subprocess when
       * it is finished or aborted.
       */
       DontCare,
       /**
        * The application is notified when the subprocess dies.
        */
       NotifyOnExit,
       /**
        * The application is suspended until the started process is finished.
        */
       Block };

  /**
   * Constructor
   */
  KProcess();

  /**
   *Destructor:
   *
   *  If the process is running when the destructor for this class
   *  is called, the child process is killed with a SIGKILL, but
   *  only if the run mode is not of type @p DontCare.
   *  Processes started as @p DontCare keep running anyway.
  */
  virtual ~KProcess();

  /**
   @deprecated

         The use of this function is now deprecated. -- Please use the
	 "operator<<" instead of "setExecutable".

	 Sets the executable to be started with this KProcess object.
	 Returns false if the process is currently running (in that
	 case the executable remains unchanged.)

	 @see operator<<

  */
  bool setExecutable(const QString& proc);


  /**
   * Sets the executable and the command line argument list for this process.
   *
   * For example, doing an "ls -l /usr/local/bin" can be achieved by:
   *  <pre>
   *  KProcess p;
   *  ...
   *  p << "ls" << "-l" << "/usr/local/bin"
   *  </pre>
   *
   **/
  KProcess &operator<<(const QString& arg);
  /**
   * Similar to previous method, takes a char *, supposed to be in locale 8 bit already.
   */
  KProcess &operator<<(const char * arg);
  /**
   * Similar to previous method, takes a QCString, supposed to be in locale 8 bit already.
   */
  KProcess &operator<<(const QCString & arg);

  /**
   * Sets the executable and the command line argument list for this process,
   * in a single method call, or add a list of arguments.
   **/
  KProcess &operator<<(const QStringList& args);

  /**
   * Clear a command line argument list that has been set by using
   * the "operator<<".
  */
  void clearArguments();

  /**
   *  Starts the process.
   *  For a detailed description of the
   *  various run modes and communication semantics, have a look at the
   *  general description of the KProcess class.
   *
   *  The following problems could cause this function to
   *    return false:
   *
   *  @li The process is already running.
   *  @li The command line argument list is empty.
   *  @li The starting of the process failed (could not fork).
   *  @li The executable was not found.
   *
   *  @param comm  Specifies which communication links should be
   *  established to the child process (stdin/stdout/stderr). By default,
   *  no communication takes place and the respective communication
   *  signals will never get emitted.
   *
   *  @return true on success, false on error
   *  (see above for error conditions)
   **/
  virtual bool start(RunMode  runmode = NotifyOnExit,
  	Communication comm = NoCommunication);

  /**
   * Stop the process (by sending it a signal).
   *
   * @param signo	The signal to send. The default is SIGTERM.
   * @return @p true if the signal was delivered successfully.
  */
  virtual bool kill(int signo = SIGTERM);

  /**
     @return @p true if the process is (still) considered to be running
  */
  bool isRunning() const;

  /** Returns the process id of the process.
   *
   * If it is called after
   * the process has exited, it returns the process id of the last
   *  child process that was created by this instance of KProcess.
   *
   *  Calling it before any child process has been started by this
   *  KProcess instance causes pid() to return 0.
   **/
  pid_t pid() const;

  /**
   * Use pid().
   * @deprecated
   */
  pid_t getPid() const { return pid(); }

  /**
   * Suspend processing of data from stdout of the child process.
   */
  void suspend();

  /**
   * Resume processing of data from stdout of the child process.
   */
  void resume();

  /**
   * @return @p true if the process has already finished and has exited
   *  "voluntarily", ie: it has not been killed by a signal.
   *
   * Note that you should check @ref KProcess::exitStatus() to determine
   * whether the process completed its task successful or not.
   */
  bool normalExit() const;

  /**
   * Returns the exit status of the process.
   *
   * Please use
   * @ref KProcess::normalExit() to check whether the process has exited
   * cleanly (i.e., @ref KProcess::normalExit() returns @p true) before calling
   * this function because if the process did not exit normally,
   * it does not have a valid exit status.
  */
  int  exitStatus() const;


  /**
   *	 Transmit data to the child process's stdin.
   *
   * KProcess::writeStdin may return false in the following cases:
   *
   *     @li The process is not currently running.
   *
   *     @li Communication to stdin has not been requested in the @ref start() call.
   *
   *     @li Transmission of data to the child process by a previous call to
   * @ref writeStdin() is still in progress.
   *
   * Please note that the data is sent to the client asynchronously,
   * so when this function returns, the data might not have been
   * processed by the child process.
   *
   * If all the data has been sent to the client, the signal
   * @ref wroteStdin() will be emitted.
   *
   * Please note that you must not free "buffer" or call @ref writeStdin()
   * again until either a @ref wroteStdin() signal indicates that the
   * data has been sent or a @ref processHasExited() signal shows that
   * the child process is no longer alive...
   **/
  bool writeStdin(const char *buffer, int buflen);

  /**
   * This causes the stdin file descriptor of the child process to be
   * closed indicating an "EOF" to the child.
   *
   * @return @p false if no communication to the process's stdin
   *  had been specified in the call to @ref start().
  */
  bool closeStdin();

  /**
   * This causes the stdout file descriptor of the child process to be
   * closed.
   *
   * @return @p false if no communication to the process's stdout
   *  had been specified in the call to @ref start().
  */
  bool closeStdout();

  /**
   * This causes the stderr file descriptor of the child process to be
   * closed.
   *
   * @return @p false if no communication to the process's stderr
   *  had been specified in the call to @ref start().
  */
  bool closeStderr();

  /**
   * Lets you see what your arguments are for debugging.
   */

  const QValueList<QCString> &args() { return arguments; }

  /**
   * Controls whether the started process should drop any
   * setuid/segid privileges or whether it should keep them
   *
   * The default is @p false : drop privileges
   */
  void setRunPrivileged(bool keepPrivileges);

  /**
   * Returns whether the started process will drop any
   * setuid/segid privileges or whether it will keep them
   */
  bool runPrivileged() const;
  
  /**
   * Modifies the environment of the process to be started.
   * This function must be called before starting the process.
   */
  void setEnvironment(const QString &name, const QString &value);

  /**
   * Changes the current working directory (CWD) of the process 
   * to be started.
   * This function must be called before starting the process.
   */
  void setWorkingDirectory(const QString &dir);

  /**
   * Specify whether to start the command via a shell or directly.
   * The default is to start the command directly.
   * If @p useShell is true @p shell will be used as shell, or
   * if shell is empty, the standard shell is used.
   * @p quote A flag indicating whether to quote the arguments.
   *
   * When using a shell, the caller should make sure that all filenames etc.
   * are properly quoted when passed as argument.
   * @see quote()
   */
  void setUseShell(bool useShell, const char *shell = 0);

  /**
   * This function can be used to quote an argument string such that
   * the shell processes it properly. This is e. g. necessary for
   * user-provided file names which may contain spaces or quotes.
   * It also prevents expansion of wild cards and environment variables.
   */
  static QString quote(const QString &arg);

  /**
   * Detaches KProcess from child process. All communication is closed.
   * No exit notification is emitted any more for the child process.
   * Deleting the KProcess will no longer kill the child process.
   * Note that the current process remains the parent process of the
   * child process.
   */
  void detach(); 



signals:

  /**
   * Emitted after the process has terminated when
   * the process was run in the @p NotifyOnExit  (==default option to
   * @ref start()) or the @ref Block mode.
   **/
  void processExited(KProcess *proc);


  /**
   * Emitted, when output from the child process has
   * been received on stdout.
   *
   *  To actually get
   * these signals, the respective communication link (stdout/stderr)
   * has to be turned on in @ref start().
   *
   * @param buffer The data received.
   * @param buflen The number of bytes that are available.
   *
   * You should copy the information contained in @p buffer to your private
   * data structures before returning from this slot.
   * Example:
   *     QString myBuf = QString::fromLatin1(buffer, buflen);
   **/
  void receivedStdout(KProcess *proc, char *buffer, int buflen);

  /**
   * Emitted when output from the child process has
   * been received on stdout.
   *
   * To actually get these signals, the respective communications link 
   * (stdout/stderr) has to be turned on in @ref start() and the 
   * @p NoRead flag should have been passed.
   *
   * You will need to explicitly call resume() after your call to start()
   * to begin processing data from the child process's stdout.  This is
   * to ensure that this signal is not emitted when no one is connected
   * to it, otherwise this signal will not be emitted.
   * 
   * The data still has to be read from file descriptor @p fd.
   **/
  void receivedStdout(int fd, int &len);


  /**
   * Emitted, when output from the child process has
   * been received on stderr.
   * To actually get
   * these signals, the respective communication link (stdout/stderr)
   * has to be turned on in @ref start().
   *
   * @param buffer The data received.
   * @param buflen The number of bytes that are available.
   *
   * You should copy the information contained in @p buffer to your private
   * data structures before returning from this slot.
  */
  void receivedStderr(KProcess *proc, char *buffer, int buflen);

  /**
   * Emitted after all the data that has been
   * specified by a prior call to @ref writeStdin() has actually been
   * written to the child process.
   **/
  void wroteStdin(KProcess *proc);


protected slots:

 /**
  * This slot gets activated when data from the child's stdout arrives.
  * It usually calls "childOutput"
  */
  void slotChildOutput(int fdno);

 /**
  * This slot gets activated when data from the child's stderr arrives.
  * It usually calls "childError"
  */
  void slotChildError(int fdno);
  /*
	Slot functions for capturing stdout and stderr of the child
  */

  /**
   * Called when another bulk of data can be sent to the child's
   * stdin. If there is no more data to be sent to stdin currently
   * available, this function must disable the QSocketNotifier "innot".
   */
  void slotSendData(int dummy);

protected:

  /**
   * Sets up the environment according to the data passed via 
   * setEnvironment(...)
   */
  void setupEnvironment();

  /**
   * The list of the process' command line arguments. The first entry
   * in this list is the executable itself.
   */
  QValueList<QCString> arguments;
  /**
   * How to run the process (Block, NotifyOnExit, DontCare). You should
   *  not modify this data member directly from derived classes.
   */
  RunMode run_mode;
  /**
   * true if the process is currently running. You should not
   * modify this data member directly from derived classes. For
   * reading the value of this data member, please use "isRunning()"
   * since "runs" will probably be made private in later versions
   * of KProcess.
   */
  bool runs;

  /**
   * The PID of the currently running process (see "getPid()").
   * You should not modify this data member in derived classes.
   * Please use "getPid()" instead of directly accessing this
   * member function since it will probably be made private in
   * later versions of KProcess.
   */
  pid_t pid_;

  /**
   * The process' exit status as returned by "waitpid". You should not
   * modify the value of this data member from derived classes. You should
   * rather use @ref exitStatus than accessing this data member directly
   * since it will probably be made private in further versions of
   * KProcess.
   */
  int status;


  /**
   * See setRunPrivileged()
   */
  bool keepPrivs;

  /*
	Functions for setting up the sockets for communication.
	setupCommunication
	-- is called from "start" before "fork"ing.
	commSetupDoneP
	-- completes communication socket setup in the parent
	commSetupDoneC
	-- completes communication setup in the child process
	commClose
	-- frees all allocated communication resources in the parent
	after the process has exited
  */

  /**
   * This function is called from "KProcess::start" right before a "fork" takes
   * place. According to
   * the "comm" parameter this function has to initialize the "in", "out" and
   * "err" data member of KProcess.
   *
   * This function should return 0 if setting the needed communication channels
   * was successful.
   *
   * The default implementation is to create UNIX STREAM sockets for the communication,
   * but you could overload this function and establish a TCP/IP communication for
   * network communication, for example.
   */
  virtual int setupCommunication(Communication comm);

  /**
   * Called right after a (successful) fork on the parent side. This function
   * will usually do some communications cleanup, like closing the reading end
   * of the "stdin" communication channel.
   *
   * Furthermore, it must also create the QSocketNotifiers "innot", "outnot" and
   * "errnot" and connect their Qt slots to the respective KProcess member functions.
   *
   * For a more detailed explanation, it is best to have a look at the default
   * implementation of "setupCommunication" in kprocess.cpp.
   */
  virtual int commSetupDoneP();

  /**
   * Called right after a (successful) fork, but before an "exec" on the child
   * process' side. It usually just closes the unused communication ends of
   * "in", "out" and "err" (like the writing end of the "in" communication
   * channel.
   */
  virtual int commSetupDoneC();


  /**
   * Immediately called after a process has exited. This function normally
   * calls commClose to close all open communication channels to this
   * process and emits the "processExited" signal (if the process was
   * not running in the "DontCare" mode).
   */
  virtual void processHasExited(int state);

  /**
   * Should clean up the communication links to the child after it has
   * exited. Should be called from "processHasExited".
   */
  virtual void commClose();


  /**
   * the socket descriptors for stdin/stdout/stderr.
   */
  int out[2];
  int in[2];
  int err[2];

  /**
   * The socket notifiers for the above socket descriptors.
   */
  QSocketNotifier *innot;
  QSocketNotifier *outnot;
  QSocketNotifier *errnot;

  /**
   * Lists the communication links that are activated for the child
   * process.  Should not be modified from derived classes.
   */
  Communication communication;

  /**
   * Called by "slotChildOutput" this function copies data arriving from the
   * child process's stdout to the respective buffer and emits the signal
   * "@ref receivedStderr".
   */
  int childOutput(int fdno);

  /**
   * Called by "slotChildOutput" this function copies data arriving from the
   * child process's stdout to the respective buffer and emits the signal
   * "@ref receivedStderr"
   */
  int childError(int fdno);

  // information about the data that has to be sent to the child:

  const char *input_data;  // the buffer holding the data
  int input_sent;    // # of bytes already transmitted
  int input_total;   // total length of input_data

  /**
   * @ref KProcessController is a friend of KProcess because it has to have
   * access to various data members.
   */
  friend class KProcessController;


private:
  /**
   * Searches for a valid shell. 
   * Here is the algorithm used for finding an executable shell:
   *
   *    @li Try the executable pointed to by the "SHELL" environment
   *    variable with white spaces stripped off
   *
   *    @li If your process runs with uid != euid or gid != egid, a shell
   *    not listed in /etc/shells will not used.
   *
   *    @li If no valid shell could be found, "/bin/sh" is used as a last resort.
   */
  QCString searchShell();

  /**
   * Used by @ref searchShell in order to find out whether the shell found
   * is actually executable at all.
   */
  bool isExecutable(const QCString &filename);

  // Disallow assignment and copy-construction
  KProcess( const KProcess& );
  KProcess& operator= ( const KProcess& );

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KProcessPrivate *d;
};

class KShellProcessPrivate;

/**
* @obsolete
*
* This class is obsolete. Use KProcess and KProcess::setUseShell(true)
* instead.
*
*   @short A class derived from @ref KProcess to start child
*   	processes through a shell.
*   @author Christian Czezakte <e9025461@student.tuwien.ac.at>
*   @version $Id$
*/
class KShellProcess: public KProcess
{
  Q_OBJECT

public:

  /**
   * Constructor
   *
   * By specifying the name of a shell (like "/bin/bash") you can override
   * the mechanism for finding a valid shell as described in KProcess::searchShell()
   */
  KShellProcess(const char *shellname=0);

  /**
   * Destructor.
   */
  ~KShellProcess();

  /**
   * Starts up the process. -- For a detailed description
   * have a look at the "start" member function and the detailed
   * description of @ref KProcess .
   */
  virtual bool start(RunMode  runmode = NotifyOnExit,
		  Communication comm = NoCommunication);

  /**
   * This function can be used to quote an argument string such that
   * the shell processes it properly. This is e. g. necessary for
   * user-provided file names which may contain spaces or quotes.
   * It also prevents expansion of wild cards and environment variables.
   */
  static QString quote(const QString &arg);

private:

  QCString shell;

  // Disallow assignment and copy-construction
  KShellProcess( const KShellProcess& );
  KShellProcess& operator= ( const KShellProcess& );

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KShellProcessPrivate *d;
};



#endif

