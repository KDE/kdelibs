/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include <qdir.h>

#include "kbuildsycoca.h"

#include <kservice.h>
#include <kmimetype.h>
#include <kbuildservicetypefactory.h>
#include <kbuildservicefactory.h>

#include <qdatastream.h>
#include <qfile.h>
#include <qtimer.h>

#include <assert.h>
#include <kinstance.h>
#include <dcopclient.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <kstddirs.h>
#include <ksavefile.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <unistd.h>

#include <stdlib.h>

KBuildSycoca::KBuildSycoca()
  : KSycoca( true )
{
}

KBuildSycoca::~KBuildSycoca()
{
}

void KBuildSycoca::build()
{
  // For each factory
  QListIterator<KSycocaFactory> factit ( *m_lstFactories );
  for (KSycocaFactory *factory = m_lstFactories->first();
       factory;
       factory = m_lstFactories->next() )
  {
    // For each resource the factory deals with
    for( QStringList::ConstIterator it1 = factory->resourceList()->begin();
         it1 != factory->resourceList()->end();
         ++it1 )
    {
      const char *resource = (*it1).ascii();

      QStringList relFiles;

      (void) KGlobal::dirs()->findAllResources( resource,
                                                QString::null,
                                                true, // Recursive!
                                                true, // uniq
                                                relFiles);
      // For each file the factory deals with.
      for( QStringList::ConstIterator it2 = relFiles.begin();
           it2 != relFiles.end();
           ++it2 )
      {
         // Create a new entry
         KSycocaEntry* entry = factory->createEntry( *it2, resource );
         if ( entry && entry->isValid() )
           factory->addEntry( entry );
      }
    }
  }
}

void KBuildSycoca::recreate()
{
  QString path = KGlobal::dirs()->saveLocation("config")+"ksycoca";

  // KSaveFile first writes to a temp file.
  // Upon close() it moves the stuff to the right place.
  KSaveFile database(path);
  if (database.status() != 0)
  {
    fprintf(stderr, "Error can't open database!\n");
    exit(-1);
  }

  m_str = database.dataStream();

  kdDebug(7021) << "Recreating ksycoca file" << endl;

  // It is very important to build the servicetype one first
  // Both are registered in KSycoca, no need to keep the pointers
  KSycocaFactory *stf = new KBuildServiceTypeFactory;
  (void) new KBuildServiceFactory(stf);

  build(); // Parse dirs
  save(); // Save database

  m_str = 0L;
  if (!database.close())
  {
     kdError(7021) << "Error writing database to " << database.name() << endl;
     return;
  }
}

void KBuildSycoca::save()
{
   // Write header (#pass 1)
   m_str->device()->at(0);

   (*m_str) << (Q_INT32) KSYCOCA_VERSION;
   KSycocaFactory * servicetypeFactory = 0L;
   KSycocaFactory * serviceFactory = 0L;
   for(KSycocaFactory *factory = m_lstFactories->first();
       factory;
       factory = m_lstFactories->next())
   {
      Q_INT32 aId;
      Q_INT32 aOffset;
      aId = factory->factoryId();
      if ( aId == KST_KServiceTypeFactory )
         servicetypeFactory = factory;
      else if ( aId == KST_KServiceFactory )
         serviceFactory = factory;
      aOffset = factory->offset();
      (*m_str) << aId;
      (*m_str) << aOffset;
   }
   (*m_str) << (Q_INT32) 0; // No more factories.
   // Write KDEDIRS
   (*m_str) << KGlobal::dirs()->kfsstnd_prefixes();

   // Write factory data....
   for(KSycocaFactory *factory = m_lstFactories->first();
       factory;
       factory = m_lstFactories->next())
   {
      factory->save(*m_str);
   }

   int endOfData = m_str->device()->at();

   // Write header (#pass 2)
   m_str->device()->at(0);

   (*m_str) << (Q_INT32) KSYCOCA_VERSION;
   for(KSycocaFactory *factory = m_lstFactories->first();
       factory;
       factory = m_lstFactories->next())
   {
      Q_INT32 aId;
      Q_INT32 aOffset;
      aId = factory->factoryId();
      aOffset = factory->offset();
      (*m_str) << aId;
      (*m_str) << aOffset;
   }
   (*m_str) << (Q_INT32) 0; // No more factories.

   kdDebug(7021) << debugString(QString("endOfData : %1").				arg(endOfData,8,16)) << endl;

   // Jump to end of database
   m_str->device()->at(endOfData);
}

bool KBuildSycoca::process(const QCString &fun, const QByteArray &/*data*/,
			   QCString &replyType, QByteArray &/*replyData*/)
{
  if (fun == "recreate()") {
    qDebug("got a recreate signal!");
    recreate();
    replyType = "void";
    return true;
  } else
    return false;
    // don't call KSycoca::process - this is for other apps, not k
}

static KCmdLineOptions options[] = {
   { "nosignal", I18N_NOOP("Don't signal applications."), 0 },
   { 0, 0, 0 }
};

static const char *appName = "kbuildsycoca";
static const char *appVersion = "1.0";

int main(int argc, char **argv)
{
   KAboutData d(appName, I18N_NOOP("KBuildSycoca"), appVersion,
                I18N_NOOP("Rebuilds the system configuration cache."),
		KAboutData::License_GPL, "(c) 1999,2000 David Faure");
   d.addAuthor("David Faure", I18N_NOOP("Author"), "faure@kde.org");

   KCmdLineArgs::init(argc, argv, &d);
   KCmdLineArgs::addCmdLineOptions(options);
   KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

   KInstance k(&d);

   DCOPClient *dcopClient = new DCOPClient();

   if (dcopClient->registerAs(appName, false) != appName)
   {
     fprintf(stderr, "%s already running!\n", appName);
     exit(0);
   }

   KBuildSycoca *sycoca= new KBuildSycoca; // Build data base
   sycoca->recreate();

   if (args->isSet("signal"))
   {
     // Notify ALL applications that have a ksycoca object, using a broadcast
     QByteArray data;
     dcopClient->send( "*", "ksycoca", "databaseChanged()", data );
   }
}



#include "kbuildsycoca.moc"
