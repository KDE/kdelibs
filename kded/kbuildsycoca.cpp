/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure <faure@kde.org>
 *  Copyright (C) 2002-2003 Waldo Bastian <bastian@kde.org>
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
#include <qeventloop.h>

#include "kbuildsycoca.h"
#include "kresourcelist.h"
#include "vfolder_menu.h"

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
#include <kapplication.h>
#include <dcopclient.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <kstandarddirs.h>
#include <ksavefile.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kcrash.h>

#include <stdlib.h>
#include <unistd.h>
#include <time.h>

typedef QDict<KSycocaEntry> myEntryDict;
typedef QValueList<KSycocaEntry::List> KSycocaEntryListList;

static Q_UINT32 newTimestamp = 0;

static KBuildServiceFactory *g_bsf = 0;
static KBuildServiceGroupFactory *g_bsgf = 0;
static KSycocaFactory *g_factory = 0;
static KCTimeInfo *g_ctimeInfo = 0;
static QDict<Q_UINT32> *g_ctimeDict = 0;
static const char *g_resource = 0;
static myEntryDict *g_entryDict = 0;
static KSycocaEntryListList *g_allEntries = 0;
static QStringList *g_changeList = 0;
static QStringList *g_allResourceDirs = 0;
static bool g_changed = false;

static VFolderMenu *g_vfolder = 0;

static const char *cSycocaPath = 0;

static bool bGlobalDatabase = 0;

void crashHandler(int)
{
   // If we crash while reading sycoca, we delete the database
   // in an attempt to recover.
   if (cSycocaPath)
      unlink(cSycocaPath);
}

static QString sycocaPath()
{
  QString path;
  
  if (bGlobalDatabase)
  {
     path = KGlobal::dirs()->saveLocation("services")+"ksycoca";
  }
  else
  {
     QCString ksycoca_env = getenv("KDESYCOCA");
     if (ksycoca_env.isEmpty())
        path = KGlobal::dirs()->saveLocation("cache")+"ksycoca";
     else
        path = QFile::decodeName(ksycoca_env);
  }

  return path;     
}

static QString oldSycocaPath()
{
  QCString ksycoca_env = getenv("KDESYCOCA");
  if (ksycoca_env.isEmpty())
     return KGlobal::dirs()->saveLocation("tmp")+"ksycoca";

  return QString::null;
}

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

KSycocaEntry *KBuildSycoca::createEntry(const QString &file)
{
   Q_UINT32 timeStamp = g_ctimeInfo->ctime(file);
   if (!timeStamp)
   {
      timeStamp = KGlobal::dirs()->calcResourceHash( g_resource, file, true);
   }
   KSycocaEntry* entry = 0;
   if (g_allEntries)
   {
      assert(g_ctimeDict);
      Q_UINT32 *timeP = (*g_ctimeDict)[file];
      Q_UINT32 oldTimestamp = timeP ? *timeP : 0;

      if (timeStamp && (timeStamp == oldTimestamp))
      {
         // Re-use old entry
         entry = g_entryDict->find(file);
         // remove from g_ctimeDict; if g_ctimeDict is not empty
         // after all files have been processed, it means
         // some files were removed since last time
         g_ctimeDict->remove( file );
      }
      else if (oldTimestamp)
      {
         g_changed = true;
         kdDebug(7021) << "modified: " << file << endl;
      }
      else 
      {
         g_changed = true;
         kdDebug(7021) << "new: " << file << endl;
      }
   }
   g_ctimeInfo->addCTime(file, timeStamp );
   if (!entry)
   {
      // Create a new entry
      entry = g_factory->createEntry( file, g_resource );
   }
   if ( entry && entry->isValid() )
   {
      g_factory->addEntry( entry, g_resource );
      return entry;
   }
   return 0;
}

void KBuildSycoca::slotCreateEntry(const QString &file, KService **service)
{
   KSycocaEntry *entry = createEntry(file);
   *service = dynamic_cast<KService *>(entry);
}

// returns false if the database is up to date
bool KBuildSycoca::build()
{
  typedef QPtrList<myEntryDict> myEntryDictList;
  myEntryDictList *entryDictList = 0;
  myEntryDict *serviceEntryDict = 0;

  entryDictList = new myEntryDictList();
  // Convert for each factory the entryList to a Dict.
  int i = 0;
  // For each factory
  for (KSycocaFactory *factory = m_lstFactories->first();
       factory;
       factory = m_lstFactories->next() )
  {
     myEntryDict *entryDict = new myEntryDict();
     if (g_allEntries)
     {
         KSycocaEntry::List list = (*g_allEntries)[i++];
         for( KSycocaEntry::List::Iterator it = list.begin();
            it != list.end();
            ++it)
         {
            entryDict->insert( (*it)->entryPath(), static_cast<KSycocaEntry *>(*it));
         }
     }
     if (factory == g_bsf)
        serviceEntryDict = entryDict;
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

  g_ctimeInfo = new KCTimeInfo(); // This is a build factory too, don't delete!!
  bool uptodate = true;
  // For all resources
  for( QStringList::ConstIterator it1 = allResources.begin();
       it1 != allResources.end();
       ++it1 )
  {
     g_changed = false;
     g_resource = (*it1).ascii();

     QStringList relFiles;

     (void) KGlobal::dirs()->findAllResources( g_resource,
                                               QString::null,
                                               true, // Recursive!
                                               true, // uniq
                                               relFiles);


     // Now find all factories that use this resource....
     // For each factory
     g_entryDict = entryDictList->first();
     for (g_factory = m_lstFactories->first();
          g_factory;
          g_factory = m_lstFactories->next(),
          g_entryDict = entryDictList->next() )
     {
        // For each resource the factory deals with
        const KSycocaResourceList *list = g_factory->resourceList();
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
               if (res.filter.search(*it3) == -1) continue;

               createEntry(*it3);
           }
        }
        if ((g_factory == g_bsf) && (strcmp(g_resource, "services") == 0))
           processGnomeVfs();
     }
     if (g_changed || !g_allEntries)
     {
        uptodate = false;
        g_changeList->append(g_resource);
     }
  }

  bool result = !uptodate || !g_ctimeDict->isEmpty();
  
  if (result)
  {
     g_resource = "apps";
     g_factory = g_bsf;
     g_entryDict = serviceEntryDict;
     g_changed = false;

     g_vfolder = new VFolderMenu;

     connect(g_vfolder, SIGNAL(newService(const QString &, KService **)),
             this, SLOT(slotCreateEntry(const QString &, KService **)));
             
     VFolderMenu::SubMenu *kdeMenu = g_vfolder->parseMenu("applications.menu", true);

     g_bsgf->addNew("/", QString::null);
     createMenu(QString::null, kdeMenu);
     
     (void) existingResourceDirs();
     *g_allResourceDirs += g_vfolder->allDirectories(); 

     disconnect(g_vfolder, SIGNAL(newService(const QString &, KService **)),
             this, SLOT(slotCreateEntry(const QString &, KService **)));

     if (g_changed || !g_allEntries)
     {
        uptodate = false;
        g_changeList->append(g_resource);
     }
  }
  
  return result;
}

void KBuildSycoca::createMenu(QString name, VFolderMenu::SubMenu *menu)
{
  for(VFolderMenu::SubMenu *subMenu = menu->subMenus.first(); subMenu; subMenu = menu->subMenus.next())
  {
     QString subName = name+subMenu->name+"/";
     g_bsgf->addNew(subName, subMenu->directoryFile);
     createMenu(subName, subMenu);
  }
  if (name.isEmpty())
     name = "/";
  for(QDictIterator<KService> it(menu->items); it.current(); ++it)
  {
     g_bsgf->addNewEntryTo(name, it.current());
  }
}

void KBuildSycoca::recreate()
{
  QString path(sycocaPath());

  // KSaveFile first writes to a temp file.
  // Upon close() it moves the stuff to the right place.
  KSaveFile database(path);
  if (database.status() != 0)
  {
    fprintf(stderr, "kbuildsycoca: ERROR creating database '%s'!\n", path.local8Bit().data());
    fprintf(stderr, "kbuildsycoca: Wrong permissions on directory? Disk full?\n");
    exit(-1);
  }

  m_str = database.dataStream();

  kdDebug(7021) << "Recreating ksycoca file (" << path << ", version " << KSYCOCA_VERSION << ")" << endl;

  // It is very important to build the servicetype one first
  // Both are registered in KSycoca, no need to keep the pointers
  KSycocaFactory *stf = new KBuildServiceTypeFactory;
  g_bsgf = new KBuildServiceGroupFactory();
  g_bsf = new KBuildServiceFactory(stf, g_bsgf);
  (void) new KBuildImageIOFactory();
  (void) new KBuildProtocolInfoFactory();

  if( build()) // Parse dirs
  {
    save(); // Save database
    if (m_str->device()->status())
      database.abort(); // Error
    m_str = 0L;
    if (!database.close())
    {
      fprintf(stderr, "kbuildsycoca: ERROR writing database '%s'!\n", database.name().local8Bit().data());
      fprintf(stderr, "kbuildsycoca: Disk full?\n");
      return;
    }
  }
  else
  {
    m_str = 0L;
    database.abort();
    kdDebug(7021) << "Database is up to date" << endl;
  }

  if (!bGlobalDatabase)
  {
    // update the timestamp file
    QString stamppath = path + "stamp";
    QFile ksycocastamp(stamppath);
    ksycocastamp.open( IO_WriteOnly );
    QDataStream str( &ksycocastamp );
    str << newTimestamp;
    str << existingResourceDirs();
    str << g_vfolder->allDirectories(); // Extra resource dirs

    // Recreate compatibility symlink
    QString oldPath = oldSycocaPath();
    if (!oldPath.isEmpty())
    {
       KTempFile tmp;
       if (tmp.status() == 0)
       {
          QString tmpFile = tmp.name();
          tmp.unlink();
          symlink(QFile::encodeName(path), QFile::encodeName(tmpFile));
          rename(QFile::encodeName(tmpFile), QFile::encodeName(oldPath));
       }
    }
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
   (*m_str) << KGlobal::locale()->language();
   (*m_str) << KGlobal::dirs()->calcResourceHash("services", "update_ksycoca", true);
   (*m_str) << (*g_allResourceDirs);

   // Write factory data....
   for(KSycocaFactory *factory = m_lstFactories->first();
       factory;
       factory = m_lstFactories->next())
   {
      factory->save(*m_str);
      if (m_str->device()->status())
         return; // error
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

bool KBuildSycoca::checkDirTimestamps( const QString& dirname, const QDateTime& stamp, bool top )
{
   if( top )
   {
      QFileInfo inf( dirname );
      if( inf.lastModified() > stamp )
         {
         kdDebug( 7021 ) << "timestamp changed:" << dirname << endl;
         return false;
         }
   }
   QDir dir( dirname );
   const QFileInfoList *list = dir.entryInfoList( QDir::DefaultFilter, QDir::Unsorted );
   if (!list)
      return true;
      
   for( QFileInfoListIterator it( *list );
        it.current() != NULL;
        ++it )
   {
      QFileInfo* fi = it.current();
      if( fi->fileName() == "." || fi->fileName() == ".." )
         continue;
      if( fi->lastModified() > stamp )
      {
         kdDebug( 7201 ) << "timestamp changed:" << fi->filePath() << endl;
         return false;
      }
      if( fi->isDir() && !checkDirTimestamps( fi->filePath(), stamp, false ))
            return false;
   }
   return true;
}

// check times of last modification of all files on which ksycoca depens,
// and also their directories
// if all of them all older than the timestamp in file ksycocastamp, this
// means that there's no need to rebuild ksycoca
bool KBuildSycoca::checkTimestamps( Q_UINT32 timestamp, const QStringList &dirs )
{
   kdDebug( 7021 ) << "checking file timestamps" << endl;
   QDateTime stamp;
   stamp.setTime_t( timestamp );
   for( QStringList::ConstIterator it = dirs.begin();
        it != dirs.end();
        ++it )
   {
      if( !checkDirTimestamps( *it, stamp, true ))
            return false;
   }
   kdDebug( 7021 ) << "timestamps check ok" << endl;
   return true;                                             
}

QStringList KBuildSycoca::existingResourceDirs()
{
   static QStringList* dirs = NULL;
   if( dirs != NULL )
       return *dirs;
   dirs = new QStringList;
   g_allResourceDirs = new QStringList;
   // these are all resources cached by ksycoca
   QStringList resources;
   resources += KBuildServiceTypeFactory::resourceTypes();
   resources += KBuildServiceGroupFactory::resourceTypes();
   resources += KBuildServiceFactory::resourceTypes();
   resources += KBuildImageIOFactory::resourceTypes();
   resources += KBuildProtocolInfoFactory::resourceTypes();
   while( !resources.empty())
   {
      QString res = resources.front();
      *dirs += KGlobal::dirs()->resourceDirs( res.latin1());
      resources.remove( res ); // remove this 'res' and all its duplicates
   }
   
   *g_allResourceDirs = *dirs;
   
   for( QStringList::Iterator it = dirs->begin();
        it != dirs->end(); )
   {
      QFileInfo inf( *it );
      if( !inf.exists() || !inf.isReadable() )
         it = dirs->remove( it );
      else
         ++it;
   }
   return *dirs;
}

static KCmdLineOptions options[] = {
   { "nosignal", I18N_NOOP("Don't signal applications."), 0 },
   { "noincremental", I18N_NOOP("Incremental update."), 0 },
   { "checkstamps", I18N_NOOP("Check file timestamps."), 0 },
   { "global", I18N_NOOP("Create global database."), 0 },
   KCmdLineLastOption
};

static const char *appName = "kbuildsycoca";
static const char *appVersion = "1.0";

class WaitForSignal : public QObject
{
public:
   ~WaitForSignal() { kapp->eventLoop()->exitLoop(); }
};

int main(int argc, char **argv)
{
   KLocale::setMainCatalogue("kdelibs");
   KAboutData d(appName, I18N_NOOP("KBuildSycoca"), appVersion,
                I18N_NOOP("Rebuilds the system configuration cache."),
                KAboutData::License_GPL, "(c) 1999-2002 KDE Developers");
   d.addAuthor("David Faure", I18N_NOOP("Author"), "faure@kde.org");
   d.addAuthor("Waldo Bastian", I18N_NOOP("Author"), "bastian@kde.org");

   KCmdLineArgs::init(argc, argv, &d);
   KCmdLineArgs::addCmdLineOptions(options);
   KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
   bGlobalDatabase = args->isSet("global");

   if (bGlobalDatabase)
   {
     setenv("KDEHOME", "-", 1);
     setenv("KDEROOTHOME", "-", 1);
   }

   KApplication::disableAutoDcopRegistration();
   KApplication k(false, false);
   k.disableSessionManagement();

   KCrash::setCrashHandler(KCrash::defaultCrashHandler);
   KCrash::setEmergencySaveFunction(crashHandler);
   KCrash::setApplicationName(QString(appName));

   // this program is in kdelibs so it uses kdelibs as catalogue
   KLocale::setMainCatalogue("kdelibs");
   // force generating of KLocale object. if not, the database will get
   // be translated
   KGlobal::locale();
   KGlobal::dirs()->addResourceType("app-reg", "share/application-registry" );

   DCOPClient *dcopClient = new DCOPClient();

   while(true)
   {
     QCString registeredName = dcopClient->registerAs(appName, false);
     if (registeredName.isEmpty())
     {
       fprintf(stderr, "Warning: %s is unable to register with DCOP.\n", appName);
       break;
     }
     else if (registeredName == appName)
     {
       break; // Go
     }
     fprintf(stderr, "Waiting for already running %s to finish.\n", appName);
       
     dcopClient->setNotifications( true );
     while (dcopClient->isApplicationRegistered(appName))
     {
       WaitForSignal *obj = new WaitForSignal;
       obj->connect(dcopClient, SIGNAL(applicationRemoved(const QCString &)),
               SLOT(deleteLater()));
       kapp->eventLoop()->enterLoop();
     }
     dcopClient->setNotifications( false );
   }
   fprintf(stderr, "%s running...\n", appName);


   bool incremental = !bGlobalDatabase && args->isSet("incremental");
   if (incremental)
   {
     QString current_language = KGlobal::locale()->language();
     QString ksycoca_language = KSycoca::self()->language();
     Q_UINT32 current_update_sig = KGlobal::dirs()->calcResourceHash("services", "update_ksycoca", true);
     Q_UINT32 ksycoca_update_sig = KSycoca::self()->updateSignature();
     
     if ((current_update_sig != ksycoca_update_sig) ||
         (current_language != ksycoca_language))
     {
        incremental = false;
        delete KSycoca::self();
     }
   }
   
   g_changeList = new QStringList;

   bool checkstamps = incremental && args->isSet("checkstamps");
   Q_UINT32 filestamp = 0;
   QStringList oldresourcedirs;
   if( checkstamps && incremental )
   {
       QString path = sycocaPath()+"stamp";
       QFile ksycocastamp(path);
       if( ksycocastamp.open( IO_ReadOnly ))
       {
           QDataStream str( &ksycocastamp );
           str >> filestamp;
           str >> oldresourcedirs;
           if( oldresourcedirs != KBuildSycoca::existingResourceDirs())
               checkstamps = false;
           if (!str.atEnd())
           {
              QStringList extraResourceDirs;
              str >> extraResourceDirs;
              oldresourcedirs += extraResourceDirs;
           }
       }
       else
       {
           checkstamps = false;
       }
   }

   newTimestamp = (Q_UINT32) time(0);

   if( !checkstamps || !KBuildSycoca::checkTimestamps( filestamp, oldresourcedirs ))
   {
      QCString qSycocaPath = QFile::encodeName(sycocaPath());
      cSycocaPath = qSycocaPath.data();

      g_allEntries = 0;
      g_ctimeDict = 0;
      if (incremental)
      {
         KSycoca *oldSycoca = KSycoca::self();
         KSycocaFactoryList *factories = new KSycocaFactoryList;
         g_allEntries = new KSycocaEntryListList;
         g_ctimeDict = new QDict<Q_UINT32>(523);

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
             g_allEntries->append( list );
         }
         delete factories; factories = 0;
         KCTimeInfo *ctimeInfo = new KCTimeInfo;
         ctimeInfo->fillCTimeDict(*g_ctimeDict);
         delete oldSycoca;
      }
      cSycocaPath = 0;

      KBuildSycoca *sycoca= new KBuildSycoca; // Build data base
      sycoca->recreate();
   }

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
