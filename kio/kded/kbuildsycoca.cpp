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
#include "kresourcelist.h"

#include <kservice.h>
#include <kmimetype.h>
#include <kbuildservicetypefactory.h>
#include <kbuildservicefactory.h>
#include <kbuildservicegroupfactory.h>
#include <kbuildimageiofactory.h>

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
  QStringList allResources;
  // For each factory
  for (KSycocaFactory *factory = m_lstFactories->first();
       factory;
       factory = m_lstFactories->next() )
  {
    // For each resource the factory deals with
    const KSycocaResourceList *list = factory->resourceList();
    if (!list) continue;

    for( KSycocaResourceList::ConstIterator it1 = list->begin();
         it1 != list->end();
         ++it1 )
    {
      KSycocaResource res = (*it1);
      if (!allResources.contains(res.resource))
         allResources.append(res.resource);
    }
  }

  // For all resources
  for( QStringList::ConstIterator it1 = allResources.begin();
       it1 != allResources.end();
       ++it1 )
  {
     const char *resource = (*it1).ascii();

     QStringList relFiles;

     (void) KGlobal::dirs()->findAllResources( resource,
                                               QString::null,
                                               true, // Recursive!
                                               true, // uniq
                                               relFiles);

     // Now find all factories that use this resource....
     // For each factory
     for (KSycocaFactory *factory = m_lstFactories->first();
          factory;
          factory = m_lstFactories->next() )
     {
        // For each resource the factory deals with
        const KSycocaResourceList *list = factory->resourceList();
        if (!list) continue;

        for( KSycocaResourceList::ConstIterator it2 = list->begin();
             it2 != list->end();
             ++it2 )
        {
           KSycocaResource res = (*it2);
           if (res.resource != (*it1)) continue;

           // For each file in the resource
           for( QStringList::ConstIterator it3 = relFiles.begin();
                it3 != relFiles.end();
                ++it3 )
           {
              // Check if file matches filter
              if (res.filter.match(*it3) == -1) continue;

              // Create a new entry
              KSycocaEntry* entry = factory->createEntry( *it3, resource );
              if ( entry && entry->isValid() )
                 factory->addEntry( entry );
           }
        }
     }
  }
}

void KBuildSycoca::recreate( KSycocaEntryListList *allEntries )
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
  KBuildServiceGroupFactory *bsgf = new KBuildServiceGroupFactory();
  (void) new KBuildServiceFactory(stf, bsgf);
  (void) new KBuildImageIOFactory();

  time_t Time1 = time(0);
  if (allEntries)
  {
     int i = 0;
     // For each factory
     for (KSycocaFactory *factory = m_lstFactories->first();
          factory;
          factory = m_lstFactories->next() )
     {
         KSycocaEntry::List list = (*allEntries)[i++];
         for( KSycocaEntry::List::Iterator it = list.begin();
            it != list.end();
            ++it)
         {
            factory->addEntry(static_cast<KSycocaEntry *>(*it));
         }
     }
  }
  else
  {
     build(); // Parse dirs
  }
  time_t Time2 = time(0);
  save(); // Save database
  time_t Time3 = time(0);

  m_str = 0L;
  if (!database.close())
  {
     kdError(7021) << "Error writing database to " << database.name() << endl;
     return;
  }
  kdDebug() << "Build = " << Time2-Time1 << "s Save = " << Time3-Time2 << "s" << endl;
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

   kdDebug(7021) << QString("endOfData : %1").arg(endOfData,8,16) << endl;

   // Jump to end of database
   m_str->device()->at(endOfData);
}

static KCmdLineOptions options[] = {
   { "nosignal", I18N_NOOP("Don't signal applications."), 0 },
   { "incremental", I18N_NOOP("Incremental update (do not use!)."), 0 },
   { 0, 0, 0 }
};

static const char *appName = "kbuildsycoca";
static const char *appVersion = "1.0";

int main(int argc, char **argv)
{
   //time_t Time1 = time(0);
   KAboutData d(appName, I18N_NOOP("KBuildSycoca"), appVersion,
                I18N_NOOP("Rebuilds the system configuration cache."),
		KAboutData::License_GPL, "(c) 1999,2000 David Faure");
   d.addAuthor("David Faure", I18N_NOOP("Author"), "faure@kde.org");

   KCmdLineArgs::init(argc, argv, &d);
   KCmdLineArgs::addCmdLineOptions(options);
   KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

   KInstance k(&d);

   // this program is in kdelibs so it uses kdelibs as catalogue
   KLocale::setMainCatalogue("kdelibs");
   // force generating of KLocale object. if not, the database will get
   // be translated
   KGlobal::locale();

   DCOPClient *dcopClient = new DCOPClient();

   if (dcopClient->registerAs(appName, false) != appName)
   {
     fprintf(stderr, "%s already running!\n", appName);
     exit(0);
   }

   bool incremental = args->isSet("incremental");
   if (incremental)
   {
     QString current_kfsstnd = KGlobal::dirs()->kfsstnd_prefixes();
     QString ksycoca_kfsstnd = KSycoca::self()->kfsstnd_prefixes();
     if (current_kfsstnd != ksycoca_kfsstnd)
     {
        incremental = false;
        delete KSycoca::self();
     }
   }

   KBuildSycoca::KSycocaEntryListList *allEntries = 0;
   if (incremental)
   {
      KSycoca *oldSycoca = KSycoca::self();
      KSycocaFactoryList *factories = new KSycocaFactoryList;
      allEntries = new KBuildSycoca::KSycocaEntryListList;

      // Must be in same order as in KBuildSycoca::recreate()!
      factories->append( new KServiceTypeFactory );
      factories->append( new KServiceGroupFactory );
      factories->append( new KServiceFactory );
      factories->append( new KImageIOFactory );

      // For each factory
      for (KSycocaFactory *factory = factories->first();
           factory;
           factory = factories->next() )
      {
          KSycocaEntry::List list;
          list = factory->allEntries();
          allEntries->append( list );
      }
      delete factories; factories = 0;
      delete oldSycoca;
   }

   KBuildSycoca *sycoca= new KBuildSycoca; // Build data base
   sycoca->recreate(allEntries);

   if (args->isSet("signal"))
   {
     // Notify ALL applications that have a ksycoca object, using a broadcast
     QByteArray data;
     dcopClient->send( "*", "ksycoca", "databaseChanged()", data );
   }
}



#include "kbuildsycoca.moc"
