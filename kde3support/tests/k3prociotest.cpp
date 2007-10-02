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
#include <kcmdlineargs.h>
#include <kcomponentdata.h>
#include <QtCore/QCoreApplication>

#include <signal.h>

#include "k3prociotest.h"



//
// A nice input for "sort"... ;- )
//
static const char txt[] = "hat\nder\nalte\nhexenmeister\nsich\ndoch\neinmal\nwegbegeben\n\
und\nnun\nsollen\nseine\ngeister\nsich\nnach\nmeinem\nwillen\nregen\nseine\nwort\nund\n\
werke\nmerkt\nich\nund\nden\nbrauch\nund\nmit\ngeistesstaerke\ntu\nich\nwunder\nauch\n";


int main(int argc, char *argv[])
{
 Dummy dummy;

 KAboutData about("kprociotest", 0, ki18n("kprociotest"), "version");
 //KCmdLineArgs::init(argc, argv, &about);
 KComponentData cData(&about);

 QCoreApplication app(argc, argv);

 printf("Welcome to the K3ProcIO Demo Application!\n");


 K3ProcIO p;

 p << "rev";

 p.connect(&p, SIGNAL(processExited(K3Process*)), &dummy, SLOT(printMessage(K3Process*)));
 p.connect(&p, SIGNAL(readReady(K3ProcIO*)), &dummy, SLOT(gotOutput(K3ProcIO*)));

 bool b;

 b = p.start();
 printf("Start returns %s\n", b ? "true" : "false");

 b = p.writeStdin(QString("Hello World!"));
 printf("writeStdin returns %s\n", b ? "true" : "false");

 b = p.writeStdin(QString("This is a test. It should come out in reverse (esrever)"));
 printf("writeStdin returns %s\n", b ? "true" : "false");

 p.closeWhenDone();

 printf("Entering man Qt event loop -- press <CTRL><C> to abort\n");
 return app.exec();
}
#include "k3prociotest.moc"
