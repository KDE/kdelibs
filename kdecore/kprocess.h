//
//  KPROCESS -- A class for handling child processes in KDE without
//  having to take care of Un*x specific implementation details
//
//  version 0.2.2, Aug 31st 1997
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
#include <qsocknot.h>


/**
  KProcess -- A class for invoking Child processes from within KDE applications

1) General usage and features

  This class allows a KDE application to start child processes without having
  to worry about UN*X signal handling issues and zombie process reaping

  Basically, this class distinguishes three different ways of running
  child processes: 

  +) "KProcess::DontCare" -- The child process is invoked and both the child
  process and the parent process continue concurrently. 

  Starting a "dont_care" child process means that the application is
  not interested in any notification to determine whether the
  child process has already exited or not.
  
  +) "KProcess::NotifyOnExit" -- The child process is invoked both the
  child and the parent process run concurrently.
  
  When the child process exits, the KProcess instance
  corresponding to it emits the Qt Signal "processExited".
  
  Since this signal is _not_ emitted from within a UN*X
  signal handler, arbitrary function calls can be made.
  
  +) "KProcess::Block" -- The child process starts and the parent
  process is suspended 
  until the child process exits. (_Really_ not recommended for programs
  with a GUI)

  KProcess also provides several functions for determining the exit status
  and the pid of the child process it represents. 

  Furthermore it is possible to supply command-line arguments to the process
  in a clean fashion (no null -- terminated stringlists and such...)

  A small usage example:

  KProcess proc;

  proc.setExecutable("my_executable");
  proc << "These" << "are" << "the" << "command" << "line" << "args";
  QApplication::connect(&proc, SIGNAL(processExited(KProcess *)), 
  pointer_to_my_object, SLOT(my_objects_slot));
  proc.start();
  
  This will start "my_executable" with the commandline arguments "These"...
  
  When the child process exits, the respective Qt signal will be emitted.
  
  2) Communication with the child process
  
  KProcess now supports communication with the child process through
  stdin/stdout/stderr.
  
  The following functions are provided for getting data from the child process
  or sending data to the child's stdin:
  
  (See below for a more in-depth description)
  
  
  bool writeStdin(char *buffer, int buflen);
  -- Transmit data to the child process's stdin.
  
  bool closeStdin();
  -- Closes the child process's stdin (which causes it to see a "feof(stdin)")
  Returns FALSE if you try to close stdin for a process that has been started
  without a communication channel to stdin.
  
  QT signals:
  
  void receivedStdout(KProcess *proc, char *buffer, int buflen);
  void receivedStderr(KProcess *proc, char *buffer, int buflen);
  -- Indicates that new data has arrived from either the
  child process's stdout or stderr.
  
  void wroteStdin(KProcess *proc);
  -- Indicates that all data that has been sent to the child process
  by a prior call to "writeStdin" has actually been transmitted to the
  client .

* @author Christian Czezatke (e9025461@student.tuwien.ac.at)
* @short Control child processes portably
* @version
*/
class KProcess : public QObject
{
  Q_OBJECT

public:

  /** enums for communication Channels to Open */
  enum Communication { NoCommunication = 0, Stdin = 1, Stdout = 2, Stderr = 4,
					   AllOutput = 6, All = 7 };

  /** various run--modes for a child process */
  enum RunMode { DontCare, NotifyOnExit, Block };

  /** Constructor */
  KProcess();

  /** Destructor
	  If the process is running when the destructor for this class
	  is called, the child process is killed with a SIGKILL, but
	  only if the run mode is not of type "DontCare". --
	  Processes started as "DontCare" keep running anyway...
  */
  virtual ~KProcess();
 
  /**
	 Sets the executable to be started with this KProcess object.
	 Returns FALSE if the process is currently running (in that
	 case the executable remains unchanged.
  */
  bool setExecutable(const char *proc);

  /** sets the command line argument list for this process */
  KProcess &operator<<(const char *arg);

  /** clears the command line argument list for this process */
  void clearArguments();

  /** starts up the process. -- For a detailed description of the
      various run modes see the general comment on "KProcess"

      returns TRUE if the process was started successfully.
      A return value of FALSE indicates that :
	  +) the process is already running
	  or
	  +) the starting of the process failed (could not fork)

      The second argument specifies which communication links should be
      established to the child process. (stdin/stdout/stderr). By default,
      no communication takes place and the respective signals will never
      get emitted.
  */
  bool start(RunMode  runmode = NotifyOnExit, Communication comm = NoCommunication);

  /**
     stops the process (by sending a SIGTERM to it). -- You may send other
     signals too of course... ;-) ) 

     returns TRUE if the signal could be delivered successfully
  */
  bool kill(int signo = SIGTERM);

  /**
     returns TRUE if the process is (still) considered to be running
  */
  bool isRunning();

  //{
  /**
     These functions allow to retrieve information about the running
     process. 
	 getPid -> returns the process id of the process (can be called
	 even _after_ the process has exited. -- Calling it before
	 executing the process will return 0
	 didExitNormally -> The process exited "volunatarily", ie: it
	 was not killed by any signal.
	 exitStatus -> The return value that the process has delivered.
	 Of course this is only a valid value for processes that
	 have finished and did exit normally.
  */
  pid_t getPid();
  bool normalExit();
  int  exitStatus(); // only valid if didExitNormally()
  //}
   
  /**
	 Transmit data to the child process's stdin. If the process is not
	 running or communication to stdin has not been turned on when
	 calling "start", FALSE is returned, otherwise TRUE.

	 Please note that the data is sent to the client asynchronousely,
	 so when this function returns, the data might not have been
	 processed by the child process. 

	 If all the data has been sent to the client, the signal
	 "wroteStdin" is emitted. 

	 Please note that you must not free "buffer" until either a
	 "wroteStdin" signal indicates that the data has been sent or ah
	 "processHasExited" signal shows that the child process is no
	 longer alive... 
  */
  bool writeStdin(char *buffer, int buflen);

  /**
     This causes the stdin file descriptor of the child process to be
	 closed indicating an "EOF" to the child.
  */
  bool closeStdin();

  signals: 

  /**
     This signal gets emitted after the process has terminated when
	 the process was run in the "NotfiyOnExit"  (=default option to
	 "start") or the "Block" mode. 
  */     
  void processExited(KProcess *proc);

  //{
  /**
     These signals get emitted, when output from the child process has
	 been received, either on stderr or on stdout. -- To actually get
	 these signals, the respective communication link (stdout/stderr)
	 has to be turned on in "start". 

     "buffer" contains the data, and "buflen" bytes are available from
	 the client. 

     You should copy the information contained in "buffer" to your private
     data structures before returning from this slot.
  */
  void receivedStdout(KProcess *proc, char *buffer, int buflen);

  void receivedStderr(KProcess *proc, char *buffer, int buflen);
  //}

  /**
     This signal gets emitted after all the data that has been
	 specified by a prior call to "writeStdin" has actually been
	 written to the child process. 
  */
  void wroteStdin(KProcess *proc);


 private slots:
 void slotChildOutput(int fdno);
  void slotChildError(int fdno);
  /*
	Slot functions for capturing stdout and stderr of the child 
  */

  void slotSendData(int dummy);
  /*
	Called when another bulk of data can be sent to the child's
	stdin.
  */

private:
  friend class KProcessController;

  void processHasExited(int state);

  int childOutput(int fdno);
  int childError(int fdno);

  int setupCommunication(Communication comm);
  int commSetupDoneP();
  int commSetupDoneC();
  void commClose();
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

  QStrList arguments; // list of the process' comand line args
  RunMode run_mode;   // how to run the process (blocking, notify, dontcare)
  bool runs;          // the process is currently executing
  char *process;      // name of the executable to be launched
  pid_t pid;          // If the process runs or has been running, it's
					  // PID is stored here 
  int status;         // the process' exit status as returned by "waitpid"

  int out[2], in[2], err[2];
  /* the socket descriptors for stdin/stdout/stderr */

  QSocketNotifier *innot, *outnot, *errnot;
  /* The socket notifiers for the above socket descriptors */

  Communication communication;		
  /* communication with the child? */
   
  // information about the data that has to be sent to the child:

  char *input_data;  // the buffer holding the data
  int input_sent;    // # of bytes already transmitted
  int input_total;   // total length of input_data
};



#endif
