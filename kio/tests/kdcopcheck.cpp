#include <kservicetypeprofile.h>
#include <kservice.h>
#include <kmimetype.h>
#include <assert.h>

#include <kservicegroup.h>
#include <kprotocolinfo.h>
#include <QtCore/QTimer>
#include <kcmdlineargs.h>
#include "kdcopcheck.h"
#include <dcopclient.h>

#include <kapplication.h>

#include <stdio.h>
#include <stdlib.h>

void debug(QString txt)
{
 fprintf(stderr, "%s\n", txt.toAscii().constData());
}

void debug(const char *txt)
{
 fprintf(stderr, "%s\n", txt);
}
void debug(const char *format, const char *txt)
{
 fprintf(stderr, format, txt);
 fprintf(stderr, "\n");
}

TestService::TestService(const QString &exec)
{
   m_exec = exec;
   proc << exec;

   proc.start();

   connect(KApplication::dcopClient(), SIGNAL(applicationRegistered(QByteArray)),
           this, SLOT(newApp(QByteArray)));
   connect(KApplication::dcopClient(), SIGNAL(applicationRemoved(QByteArray)),
           this, SLOT(endApp(QByteArray)));
   connect(&proc, SIGNAL(finished(int,QProcess::ExitStatus)),
           this, SLOT(appExit()));

   QTimer::singleShot(20*1000, this, SLOT(stop()));
   result = KService::DCOP_None;
}

void TestService::newApp(const QByteArray &appId)
{
   QString id = appId;
   if (id == m_exec)
   {
      result = KService::DCOP_Unique;
      stop();
   }
   else if (id.startsWith(m_exec))
   {
      result = KService::DCOP_Multi;
      stop();
   }
   qWarning("Register %s", appId.data());
}

void TestService::endApp(const QByteArray &appId)
{
   qWarning("Unegister %s", appId.data());
}

void TestService::appExit()
{
   qWarning("Exit");
}

void TestService::stop()
{
   qApp->exit_loop();
}

int TestService::exec()
{
   qApp->enter_loop();
   return result;
}

int main(int argc, char *argv[])
{
   putenv("IGNORE_SYCOCA_VERSION=true");
   KCmdLineArgs::init( argc,argv, "whatever", 0, qi18n("whatever"), 0);
   KApplication k( false/*noGUI*/ );// KMessageBox needs KApp for makeStdCaption

   k.dcopClient()->setNotifications(true);

   KService::List list = KService::allServices();

   qWarning("I found %d services.", list.count());
   int i = 0;
   for(KService::List::ConstIterator it = list.begin(); it != list.end(); ++it)
   {
      if (((*it)->DCOPServiceType() == KService::DCOP_None) &&
          !(*it)->entryPath().startsWith("SuSE") &&
           (*it)->hasServiceType("Application"))
      {
         if ((*it)->exec().startsWith((*it)->desktopEntryName()))
         {
            i++;

            TestService *test = new TestService((*it)->desktopEntryName());
            int n = test->exec();
            delete test;

            QString result;
            if (n == KService::DCOP_None)
               result = "None";
            else if (n == KService::DCOP_Unique)
               result = "Unique";
            else if (n == KService::DCOP_Multi)
               result = "Multi";

            qWarning("%s %s", (*it)->entryPath().toLatin1().constData(),
                              result.toLatin1().constData());
         }
      }
   }
   qWarning("%d left after filtering.", i);
}

