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
#include <qstrlist.h>
#include <qobject.h>
#include <qsocketnotifier.h>


/**
  @short Child process invocation, monitoring and control.
  @author Christian Czezakte e9025461@student.tuwien.ac.at

1) General usage and features

  This class allows a KDE application to start child processes without having
  to worry about UN*X signal handling issues and zombie process reaping

  Basically, this class distinguishes three different ways of running
  child processes: 

  @li "KProcess::DontCare" -- The child process is invoked and both the child
  process and the parent process continue concurrently. 

  Starting a "DontCare" child process means that the application is
  not interested in any notification to determine whether the
  child process has already exited or not.
  
  @li "KProcess::NotifyOnExit" -- The child process is invoked both the
  child and the parent process run concurrently.
  
  When the child process exits, the KProcess instance
  corresponding to it emits the Qt Signal "processExited".
  
  Since this signal is _not_ emitted from within a UN*X
  signal handler, arbitrary function calls can be made.
  
  @li "KProcess::Block" -- The child process starts and the parent process
  is suspended until the child process exits. (_Really_ not recommended
  for programs with a GUI)

  KProcess also provides several functions for determining the exit status
  and the pid of the child process it represents. 

  Furthermore it is possible to supply command-line arguments to the process
  in a clean fashion (no null -- terminated stringlists and such...)

  A small usage example:
  <pre>
  KProcess proc;

  proc << "my_executable";
  proc << "These" << "are" << "the" << "command" << "line" << "args";
  QApplication::connect(&proc, SIGNAL(processExited(KProcess *)), 
                        pointer_to_my_object, SLOT(my_objects_slot));
  proc.start();
  </pre>  

  This will start "my_executable" with the commandline arguments "These"...
  
  When the child process exits, the respective Qt signal will be emitted.
  
  2) Communication with the child process
  
  KProcess supports communication with the child process through
  stdin/stdout/stderr.
  
  The following functions are provided for getting data from the child
  process or sending data to the child's stdin (For more information,
  have a look at the documentation of each function):

  bool writeStdin(char *buffer, int buflen);
  -- Transmit data to the child process's stdin.
  
  bool closeStdin();
  -- Closes the child process's stdin (which causes it to see a "feof(stdin)")
  Returns false if you try to close stdin for a process that has been started
  without a communication channel to stdin.
  
  QT signals:
  
  void receivedStdout(KProcess *proc, char *buffer, int buflen);
  void receivedStderr(KProcess *proc, char *buffer, int buflen);
  -- Indicates that new data has arrived from either the
  child process's stdout or stderr.
  
  void wroteStdin(KProcess *proc);
  -- Indicates that all data that has been sent to the child process
  by a prior call to "writeStdin" has actually been transmitted to the
  client 
*/
class KProcess : public QObject
{
  Q_OBJECT

public:

  /** 
   * Modes in which the communication channel can be opened.
	* If communication for more than one channel is required,
	* the values have to be or'ed together, for example to get
	* communication with stdout as well as with stdin, you would
	* specify "Stdin | Stdout"
  */
  enum Communication { NoCommunication = 0, Stdin = 1, Stdout = 2, Stderr = 4,
					   AllOutput = 6, All = 7 };

  /** 
   * Run-modes for a child process.       
   * @see KProcess
  */
  enum RunMode { DontCare, NotifyOnExit, Block };

  /** Constructor */
  KProcess();

  /**
         Destructor:

	  If the process is running when the destructor for this class
	  is called, the child process is killed with a SIGKILL, but
	  only if the run mode is not of type "DontCare". --
	  Processes started as "DontCare" keep running anyway...
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
   * Sets the executable and the command line argument list for this process 
      
      For example, doning a "ls -l /usr/local/bin" can be achieved by:
      <pre>
      KProcoess p;
      ...
      p << "ls" << "-l" << "/usr/local/bin"
      </pre>

   */
  KProcess &operator<<(const QString& arg);

  /** 
	Clears a command line argument list that has been set by using
	the "operator<<".
  */
  void clearArguments();

  /** Starts up the process. -- For a detailed description of the 
      various run modes and communication semantics, have a look at the 
      general description of the KProcess class.

      The following problems could cause KProcess:start" to return false:

	  @li the process is already running
	  @li the command line argument list is empty 
	  @li  the starting of the process failed (could not fork)

      @param comm	 Specifies which communication links should be
      established to the child process. (stdin/stdout/stderr). By default,
      no communication takes place and the respective communication
      signals will never get emitted.

      @return true on success, false on error (see above for error conditions)
  */
  virtual bool start(RunMode  runmode = NotifyOnExit, 
  	Communication comm = NoCommunication);

  /**
     	Stops the process (by sending it a signal).

	@param signo	The signal to send. The default is SIGTERM.
	@return true if the signal was delivered successfully.
  */
  virtual bool kill(int signo = SIGTERM);

  /**
     @return true if the process is (still) considered to be running
  */
  bool isRunning();

  /** Returns the process id of the process. If it is called after
      the process has exited, it returns the process id of the last
      child process that was created by this instance of KProcess.

      Calling it before any child process has been started by this
      KProcess instance causes getPid to return 0
  */    
  pid_t getPid();


  /** 
	@return true if the process has already finished and has exited
	"voluntarily", ie: it has not been killed by a signal.
  */
  bool normalExit();

  /** 
	The exit status of the process. Please use
	@ref KProcess::normalExit to check whether the process has exited
	cleanly (KProcess::normalExit returning true) before calling
	this function because if the process did not exit normally,
	it does not have a valid exit status.
  */
  int  exitStatus(); 

   
  /**
	 Transmit data to the child process's stdin. KProcess::writeStdin
         may return false in the following cases:

         +) The process is not currently running

	 +) Communication to stdin has not been requested in the "start" call

         +) transmission of data to the child process by a previous call to 
	 "writeStdin" is still in progress. 

	 Please note that the data is sent to the client asynchronousely,
	 so when this function returns, the data might not have been
	 processed by the child process. 

	 If all the data has been sent to the client, the signal
	 "wroteStdin" will be emitted. 

	 Please note that you must not free "buffer" or call "writeStdin"
	 again until either a "wroteStdin" signal indicates that the
	 data has been sent or a "processHasExited" signal shows that
	 the child process is no longer alive...
  */
  bool writeStdin(const char *buffer, int buflen);

  /**
     This causes the stdin file descriptor of the child process to be
	 closed indicating an "EOF" to the child. This function will
	 return false if:

	 +) No communication to the process's stdin has been specified
	 in the "start" call.
  */
  bool closeStdin();

  signals: 

  /**
     This signal gets emitted after the process has terminated when
	 the process was run in the "NotfiyOnExit"  (=default option to
	 "start") or the "Block" mode. 
  */     
  void processExited(KProcess *proc);

 
  /**
     These signals get emitted, when output from the child process has
	 been received on stdout. -- To actually get
	 these signals, the respective communication link (stdout/stderr)
	 has to be turned on in "start". 

     "buffer" contains the data, and "buflen" bytes are available from
	 the client. 

     You should copy the information contained in "buffer" to your private
     data structures before returning from this slot.
  */
  void receivedStdout(KProcess *proc, char *buffer, int buflen);


  /**
     These signals get emitted, when output from the child process has
	 been received on stderr. -- To actually get
	 these signals, the respective communication link (stdout/stderr)
	 has to be turned on in "start". 

     "buffer" contains the data, and "buflen" bytes are available from
	 the client. 

     You should copy the information contained in "buffer" to your private
     data structures before returning from this slot.
  */
  void receivedStderr(KProcess *proc, char *buffer, int buflen);

  /**
     This signal gets emitted after all the data that has been
	 specified by a prior call to "writeStdin" has actually been
	 written to the child process. 
  */
  void wroteStdin(KProcess *proc);


protected slots:

 /**
   This slot gets activated when data from the child's stdout arrives.
   It usually calls "childOutput"
  */
  void slotChildOutput(int fdno);

 /**
   This slot gets activated when data from the child's stderr arrives.
   It usually calls "childError"
  */
  void slotChildError(int fdno);
  /*
	Slot functions for capturing stdout and stderr of the child 
  */


  /**
	Called when another bulk of data can be sent to the child's
	stdin. If there is no more data to be sent to stdin currently
	available, this function must disable the QSocketNotifier "innot".
  */
  void slotSendData(int dummy);

protected:

  /**
     The list of the process' command line arguments. The first entry
     in this list is the executable itself.
  */
  QStrList arguments;
  /**
     How to run the process (blocking, notify_on_exit, dontcare). You should
     not modify this data member directly from derived classes.
  */
  RunMode run_mode;
  /** 
     true if the process is currently running. You should not 
     modify this data member directly from derived classes. For
     reading the value of this data member, please use "isRunning()"
     since "runs" will probably be made private in later versions
     of KProcess.
  */
  bool runs;

  /** 
      The PID of the currently running process (see "getPid()").
      You should not modify this data member in derived classes.
      Please use "getPid()" instead of directly accessing this
      member function since it will probably be made private in
      later versions of KProcess.
  */

  pid_t pid;

  /** The process' exit status as retrned by "waitpid". You should not 
      modify the value of this data member from derived classes. You should
      rather use "getStatus()" than accessing this data member directly
      since it will probably be made private in further versions of
      KProcess.
  */
  int status;


  /*
	Functions for setting up the sockets for communication.
	setupCommunication 
	-- is called from "start" before "fork"ing.
	commSetupDoneP
	-- completes communcation socket setup in the parent
	commSetupDoneC
	-- completes communication setup in the child process
	commClose
	-- frees all allocated communication ressources in the parent
	after the process has exited
  */

  /**
    This function is called from "KProcess::start" right before a "fork" takes 
    place. According to
    the "comm" parameter this function has to initialize the "in", "out" and
    "err" data member of KProcess.

    This function should return 0 if setting the needed communication channels
    was successful. 

    The default implementation is to create UNIX STREAM sockets for the communication,
    but you could overload this function and establish a TCP/IP communication for
    network communication, for example. 
  */      
  virtual int setupCommunication(Communication comm);

  /**
     Called right after a (successful) fork on the parent side. This function
     will usually do some communications cleanup, like closing the reading end
     of the "stdin" communication channel.

     Furthermore, it must also create the QSocketNotifiers "innot", "outnot" and
     "errnot" and connect their Qt slots to the respective KProcess member functions.

     For a more detailed explanation, it is best to have a look at the default
     implementation of "setupCommunication" in @ref kprocess.cpp.
  */
  virtual int commSetupDoneP();
 
  /**
     Called right after a (successful) for, but before an "exec" on the child
     process' side. It usually just closes the unused communication ends of
     "in", "out" and "err" (like the writing end of the "in" communication
     channel.
  */
  virtual int commSetupDoneC();


  /**
     Immediately called after a process has exited. This function normally
     calls commClose to close all open communication channels to this
     process and emits the "processExited" signal (if the process was
     not running in the "DontCare" mode).
  */ 
  virtual void processHasExited(int state); 
  
  /**
     Should clean up the communication links to the child after it has
     exited. Should be called from "processHasExited".
  */
  virtual void commClose();


  /**
   * the socket descriptors for stdin/stdout/stderr.
   */
  int out[2], in[2], err[2]; 

  /**
   * The socket notifiers for the above socket descriptors.
   */
  QSocketNotifier *innot, *outnot, *errnot; 

  /**
     Lists the communication links that are activated for the child
     process.  Should not be modified from derived classes.
  */ 
  Communication communication;

  /**
     Called by "slotChildOutput" this function copies data arriving from the
     child process's stdout to the respective buffer and emits the signal
     "@ref receivedStderr".
  */
  int childOutput(int fdno);

  /**
     Called by "slotChildOutput" this function copies data arriving from the
     child process's stdout to the respective buffer and emits the signal
     "@ref receivedStderr"
  */
  int childError(int fdno);
   
  // information about the data that has to be sent to the child:

  const char *input_data;  // the buffer holding the data
  int input_sent;    // # of bytes already transmitted
  int input_total;   // total length of input_data

  /**
    @ref KProcessController is a friend of KProcess because it has to have
    access to various data members.
  */
  friend class KProcessController;


private:
  // Disallow assignment and copy-construction
  KProcess( const KProcess& );
  KProcess& operator= ( const KProcess& );
};

/**
* This class is similar to @ref KProcess. The only difference is that
* KShellProcess runs the specified executable through a UN*X shell so
* that standard shell mechanisms like wildcard matching, use of pipes
* and environment variable expansion will work.

* For example, you could run commands like the following through
* KShellProcess:

* <pre>
*   ls ~/HOME/ *.lyx | sort | uniq |wc -l
* </pre>

* KShellProcess tries really hard to find a valid executable shell. Here
* is the algorithm used for finding an executable shell:

*    @li Try to use executable pointed to by the "SHELL" environment
*    variable

*    @li Try the executable pointed to by the "SHELL" environment
*    variable with whitespaces stripped off

*    @li "/bin/sh" as a last ressort.

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
      Constructor

      By specifying the name of a shell (like "/bin/bash") you can override
      the mechanism for finding a valid shell as described in the detailed
      description of this class.
  */
  KShellProcess(const char *shellname=0);

  /**
   * Descructor.
   */
  ~KShellProcess();

  /** 
    Starts up the process. -- For a detailed description
    have a look at the "start" member function and the detailed
    description of @ref KProcess .
  */
  virtual bool start(RunMode  runmode = NotifyOnExit, 
		  Communication comm = NoCommunication);

private:

  /** 
   * Searches for a valid shell. See the general description of this
   * class for information on how the search is actually performed.
  */
  char *searchShell();

  /** 
   * Used by @ref searchShell in order to find out whether the shell found
   * is actually executable at all.
  */
  bool isExecutable(const char *fname);

  const char *shell;

  // Disallow assignment and copy-construction
  KShellProcess( const KShellProcess& );
  KShellProcess& operator= ( const KShellProcess& );
};



#endif
