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
#include <kbuildprotocolinfofactory.h>
#include <kctimefactory.h>
#include <kdatastream.h>

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
#include <unistd.h>

static Q_UINT32 newTimestamp = 0;

static KBuildServiceFactory *g_bsf = 0;

static QStringList *g_changeList = 0;

KBuildSycoca::KBuildSycoca()
  : KSycoca( true )
{
}

KBuildSycoca::~KBuildSycoca()
{

}

void KBuildSycoca::processGnomeVfs()
{
   QString file = locate("app-reg", "gnome-vfs.applications");
   if (file.isEmpty())
   {
//      kdDebug(7021) << "gnome-vfs.applications not found." << endl;
      return;
   }

   QString app;

   char line[1024*64];
 
   FILE *f = fopen(QFile::encodeName(file), "r");
   while (!feof(f))
   {
      if (!fgets(line, sizeof(line)-1, f))
      {
        break;
      }

      if (line[0] != '\t')
      {
          app = QString::fromLatin1(line);
          app.truncate(app.length()-1);
      }
      else if (strncmp(line+1, "mime_types=", 11) == 0)
      {
          QString mimetypes = QString::fromLatin1(line+12);
          mimetypes.truncate(mimetypes.length()-1);
          mimetypes.replace(QRegExp("\\*"), "all");
          KService *s = g_bsf->findServiceByName(app);
          if (!s)
             continue;

          QStringList &serviceTypes = s->accessServiceTypes();
          if (serviceTypes.count() <= 1)
          {
             serviceTypes += QStringList::split(',', mimetypes);
//             kdDebug(7021) << "Adding gnome mimetypes for '" << app << "'.\n";
//             kdDebug(7021) << "ServiceTypes=" << s->serviceTypes().join(":") << endl;
          }
      }
   }
   fclose( f );
}

// returns false if the database is up to date
bool KBuildSycoca::build(KSycocaEntryListList *allEntries,
                         QDict<Q_UINT32> *ctimeDict)
{
  typedef QDict<KSycocaEntry> myEntryDict;
  typedef QList<myEntryDict> myEntryDictList;
  myEntryDictList *entryDictList = 0;

  entryDictList = new myEntryDictList();
  // Convert for each factory the entryList to a Dict.
  int i = 0;
  // For each factory
  for (KSycocaFactory *factory = m_lstFactories->first();
       factory;
       factory = m_lstFactories->next() )
  {
     myEntryDict *entryDict = new myEntryDict();
     if (allEntries)
     {
         KSycocaEntry::List list = (*allEntries)[i++];
         for( KSycocaEntry::List::Iterator it = list.begin();
            it != list.end();
            ++it)
         {
            entryDict->insert( (*it)->entryPath(), static_cast<KSycocaEntry *>(*it));
         }
     }
     entryDictList->append(entryDict);
  }

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

  KCTimeInfo *ctimeInfo = new KCTimeInfo();
  bool uptodate = true;
  // For all resources
  for( QStringList::ConstIterator it1 = allResources.begin();
       it1 != allResources.end();
       ++it1 )
  {
     bool changed = false;
     const char *resource = (*it1).ascii();

     QStringList relFiles;

     (void) KGlobal::dirs()->findAllResources( resource,
                                               QString::null,
                                               true, // Recursive!
                                               true, // uniq
                                               relFiles);


     // Now find all factories that use this resource....
     // For each factory
     myEntryDict *entryDict = entryDictList->first();
     for (KSycocaFactory *factory = m_lstFactories->first();
          factory;
          factory = m_lstFactories->next(),
          entryDict = entryDictList->next() )
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

               Q_UINT32 timeStamp = ctimeInfo->ctime(*it3);
               if (!timeStamp)
               {
                   timeStamp = KGlobal::dirs()->calcResourceHash( resource, *it3, true);
               }
               KSycocaEntry* entry = 0;
               if (allEntries)
               {
                   assert(ctimeDict);
                   Q_UINT32 *timeP = (*ctimeDict)[*it3];
                   Q_UINT32 oldTimestamp = timeP ? *timeP : 0;

                   if (timeStamp && (timeStamp == oldTimestamp))
                   {
                      // Re-use old entry
                      entry = entryDict->find(*it3);
                      // remove from ctimeDict; if ctimeDict is not empty
                      // after all files have been processed, it means
                      // some files were removed since last time
                      ctimeDict->remove( *it3 );
                   }
                   else if (oldTimestamp)
                   {
                      changed = true;
                      kdDebug(7021) << "modified: " << (*it3) << endl;
                   }
                   else 
                   {
                      changed = true;
                      kdDebug(7021) << "new: " << (*it3) << endl;
                   }
               }
               ctimeInfo->addCTime(*it3, timeStamp );
               if (!entry)
               {
                   // Create a new entry
                   entry = factory->createEntry( *it3, resource );
               }
               if ( entry && entry->isValid() )
                  factory->addEntry( entry, resource );
           }
        }
        if ((factory == g_bsf) && (strcmp(resource, "apps") == 0))
           processGnomeVfs();
     }
     if (changed || !allEntries)
     {
        uptodate = false;
        g_changeList->append(resource);
     }
  }
  return !uptodate || !ctimeDict->isEmpty();
}

void KBuildSycoca::recreate( KSycocaEntryListList *allEntries, QDict<Q_UINT32> *ctimeDict)
{
  QString path = KGlobal::dirs()->saveLocation("tmp")+"ksycoca";

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
  g_bsf = new KBuildServiceFactory(stf, bsgf);
  (void) new KBuildImageIOFactory();
  (void) new KBuildProtocolInfoFactory();

  if( build(allEntries, ctimeDict)) // Parse dirs
  {
    save(); // Save database
    m_str = 0L;
    if (!database.close())
    {
      kdError(7021) << "Error writing database to " << database.name() << endl;
      return;
    }
  }
  else
  {
    m_str = 0L;
    database.abort();
    kdDebug(7021) << "Database is up to date" << endl;
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
   (*m_str) << newTimestamp;

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

   // Jump to end of database
   m_str->device()->at(endOfData);
}

static KCmdLineOptions options[] = {
   { "nosignal", I18N_NOOP("Don't signal applications."), 0 },
   { "incremental", I18N_NOOP("Incremental update."), 0 },
   { 0, 0, 0 }
};

static const char *appName = "kbuildsycoca";
static const char *appVersion = "1.0";

int main(int argc, char **argv)
{
   KLocale::setMainCatalogue("kdelibs");
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
   KGlobal::dirs()->addResourceType("app-reg", "share/application-registry" );

   DCOPClient *dcopClient = new DCOPClient();

   QCString registeredName = dcopClient->registerAs(appName, false);
   if (registeredName.isEmpty())
   {
     fprintf(stderr, "Warning: %s is unable to register with DCOP.\n", appName);
   }
   else if (registeredName != appName)
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

   g_changeList = new QStringList;

   KBuildSycoca::KSycocaEntryListList *allEntries = 0;
   QDict<Q_UINT32> *ctimeDict = 0;
   if (incremental)
   {
      KSycoca *oldSycoca = KSycoca::self();
      KSycocaFactoryList *factories = new KSycocaFactoryList;
      allEntries = new KBuildSycoca::KSycocaEntryListList;
      ctimeDict = new QDict<Q_UINT32>(523);

      // Must be in same order as in KBuildSycoca::recreate()!
      factories->append( new KServiceTypeFactory );
      factories->append( new KServiceGroupFactory );
      factories->append( new KServiceFactory );
      factories->append( new KImageIOFactory );
      factories->append( new KProtocolInfoFactory );

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
      KCTimeInfo *ctimeInfo = new KCTimeInfo;
      ctimeInfo->fillCTimeDict(*ctimeDict);
      delete oldSycoca;
   }

   newTimestamp = (Q_UINT32) time(0);

   KBuildSycoca *sycoca= new KBuildSycoca; // Build data base
   sycoca->recreate(allEntries, ctimeDict);

   if (args->isSet("signal"))
   {
     // Notify ALL applications that have a ksycoca object, using a broadcast
     QByteArray data;
     QDataStream stream(data, IO_WriteOnly);
     stream << *g_changeList;
     dcopClient->send( "*", "ksycoca", "notifyDatabaseChanged(QStringList)", data );
   }
}

#include "kbuildsycoca.moc"
