//
//  MAIN -- a little demo of the capabilities of the "K3Process" class
//
//  version 0.2, Aug 2nd 1997
//
//  Copyright 1997 Christian Czezatke <e9025461@student.tuwien.ac.at>
//


#include "k3process.h"

#include <stdio.h>
#include <string.h>
#include <kaboutdata.h>
#include <kcomponentdata.h>
#include <QtCore/QCoreApplication>

#include <signal.h>

#include "k3processtest.h"

#define PROCNO 10


//
// A nice input for "sort"... ;- )
//
static const char txt[] = "hat\nder\nalte\nhexenmeister\nsich\ndoch\neinmal\nwegbegeben\n\
und\nnun\nsollen\nseine\ngeister\nsich\nnach\nmeinem\nwillen\nregen\nseine\nwort\nund\n\
werke\nmerkt\nich\nund\nden\nbrauch\nund\nmit\ngeistesstaerke\ntu\nich\nwunder\nauch\n";


int main(int argc, char *argv[])
{
 K3Process p1, p2, p3, p4;
 Dummy dummy;
 KAboutData about("kprocesstest", 0, ki18n("kprocesstest"), "version");
 KComponentData cData(&about);
 //KCmdLineArgs::init(argc, argv, &about);
 //KApplication app;
 QCoreApplication app(argc, argv);

 printf("Welcome to the K3Process Demo Application!\n");

 //
 // The kghostview demo -- Starts a kghostview instance blocking. -- After
 // kghostview has exited, kghostview is restarted non-blocking. When the process exits,
 // the signal "processExited" will be emitted.
 //

 p1 << "kghostview";
 QObject::connect(&p1, SIGNAL(processExited(K3Process *)),  &dummy, SLOT(printMessage(K3Process *)));

 printf("starting kghostview blocking (close to continue)\n");
 p1.start(K3Process::Block);
 printf("restarting kghostview non blocking\n");
 p1.start();


 //
 // A konsole with tcsh to demonstrate how to pass command line options to a process
 // with "K3Process" (is run blocking)
 //

 printf("Starting konsole with /bin/tcsh as shell (close to continue)\n");
 p2 << "konsole" << "-e" << "/bin/tcsh";
 p2.setWorkingDirectory("/tmp");
 QObject::connect(&p2, SIGNAL(processExited(K3Process *)),  &dummy, SLOT(printMessage(K3Process *)));
 p2.start(K3Process::Block);

 //
 // Getting the output from a process. "ls" with parameter "-l" is called and it output is captured
 //

 p3 << "ls" << "-l";
 QObject::connect(&p3, SIGNAL(processExited(K3Process *)),
		  &dummy, SLOT(printMessage(K3Process *)));

 QObject::connect(&p3, SIGNAL(receivedStdout(K3Process *, char *, int)),
		  &dummy, SLOT(gotOutput(K3Process *, char *, int)));
 QObject::connect(&p3, SIGNAL(receivedStderr(K3Process *, char *, int)),
		  &dummy, SLOT(gotOutput(K3Process *, char *, int)));

 p3.start(K3Process::NotifyOnExit, K3Process::AllOutput);


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

 p4 << "sort";
 QObject::connect(&p4, SIGNAL(processExited(K3Process *)),
		  &dummy, SLOT(printMessage(K3Process *)));

 QObject::connect(&p4, SIGNAL(receivedStdout(K3Process *, char *, int)),
		  &dummy, SLOT(gotOutput(K3Process *, char *, int)));
 QObject::connect(&p4, SIGNAL(receivedStderr(K3Process *, char *, int)),
		  &dummy, SLOT(gotOutput(K3Process *, char *, int)));

 QObject::connect(&p4, SIGNAL(wroteStdin(K3Process *)),
		  &dummy, SLOT(outputDone(K3Process *)));

 p4.start(K3Process::NotifyOnExit, K3Process::All);
 printf("after p4.start");
 p4.writeStdin(txt, strlen(txt));

 printf("Entering man Qt event loop -- press <CTRL><C> to abort\n");
 return app.exec();
}
#include "k3processtest.moc"
