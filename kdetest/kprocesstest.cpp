//
//  MAIN -- a little demo of the capabilities of the "KProcess" class
//
//  version 0.2, Aug 2nd 1997
//
//  (C) Christian Czezatke
//  e9025461@student.tuwien.ac.at
//


#include "kprocess.h"
#include "kprocesstest.h"

#include <stdio.h>
#include <string.h>
#include <kapp.h>

#include <signal.h>

#include "kprocesstest.moc"

#define PROCNO 10


//
// A nice input for "sort"... ;- )
//
static char *txt= "hat\nder\nalte\nhexenmeister\nsich\ndoch\neinmal\nwegbegeben\n\
und\nnun\nsollen\nseine\ngeister\nsich\nnach\nmeinem\nwillen\nregen\nseine\nwort\nund\n\
werke\nmerkt\nich\nund\nden\nbrauch\nund\nmit\ngeistesstaerke\ntu\nich\nwunder\nauch\n";


void main(int argc, char *argv[])
{
 KProcess p1, p2, p3, p4;
 Dummy dummy; 
 KApplication app(argc, argv);


 printf("Welcome to the KProcess Demo Application!\n");

 if (strcmp("1.2",qVersion()))
   printf("Qt Version %s\n", qVersion());
 else
   printf("Qt 1.2 -- QSocketNotifier Workaround enabled!\n");

 //
 // The kghostview demo -- Starts a kghostview instance blocking. -- After
 // kghostview has exited, kghostview is restarted non-blocking. When the process exits, 
 // the signal "processExited" will be emitted.
 //

 p1.setExecutable("kghostview");
 QObject::connect(&p1, SIGNAL(processExited(KProcess *)),  &dummy, SLOT(printMessage(KProcess *)));
 
 printf("starting kghostview blocking (close to continue)\n");
 p1.start(KProcess::Block);
 printf("restarting kghostview non blocking\n");
 p1.start();


 //
 // A kvt doing a "ls-l" to demonstrate how to pass command line options to a process
 // with "KProcess" (is run non blocking too) -- This will run only a short time
 // so all you might get to see is a "kvt" popping up and disappearing again immediatedly
 //

 printf("Starting kvt for doing a ls (will only run a short time)\n");
 p2.setExecutable("kvt");
 p2 << "-e" << "-ls -l";
 QObject::connect(&p2, SIGNAL(processExited(KProcess *)),  &dummy, SLOT(printMessage(KProcess *)));
 p2.start();

 //
 // Getting the output from a process. "ls" with parameter "-l" is called and it output is captured
 //

 p3.setExecutable("ls");
 p3 << "-l";
 QObject::connect(&p3, SIGNAL(processExited(KProcess *)), 
		  &dummy, SLOT(printMessage(KProcess *)));

 QObject::connect(&p3, SIGNAL(receivedStdout(KProcess *, char *, int)),
		  &dummy, SLOT(gotOutput(KProcess *, char *, int)));
 QObject::connect(&p3, SIGNAL(receivedStderr(KProcess *, char *, int)),
		  &dummy, SLOT(gotOutput(KProcess *, char *, int)));

 p3.start(KProcess::NotifyOnExit, KProcess::AllOutput);
 

 //
 // An even more advanced example of communicating with a child proces. -- A "sort" command
 // is started. After it has been started a list of words (as stored in "txt") is written
 // to its stdin. When the sort command has absorbed all its input it will emit the signal
 // "inputSent". -- This signal is connected to "outputDone" in the Dummy object. 
 // 
 // "OutputDone" will do a "sendEof" to p4. -- This will cause "sort" to perform its task.
 // The output of sort is then captured once more by connecting to the signal "outputWaiting"
 //
 //

 p4.setExecutable("sort");
 QObject::connect(&p4, SIGNAL(processExited(KProcess *)), 
		  &dummy, SLOT(printMessage(KProcess *)));

 QObject::connect(&p4, SIGNAL(receivedStdout(KProcess *, char *, int)),
		  &dummy, SLOT(gotOutput(KProcess *, char *, int)));
 QObject::connect(&p4, SIGNAL(receivedStderr(KProcess *, char *, int)),
		  &dummy, SLOT(gotOutput(KProcess *, char *, int)));

 QObject::connect(&p4, SIGNAL(wroteStdin(KProcess *)),
		  &dummy, SLOT(outputDone(KProcess *)));

 p4.start(KProcess::NotifyOnExit, KProcess::All);
 debug("after p4.start");
 p4.writeStdin(txt, strlen(txt)); 

 printf("Entering man Qt event loop -- press <CTRL><C> to abort\n");
 app.exec();
}
