// -*- c-basic-offset: 3 -*-
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kbuildsycoca.h"
#include "ksycoca_p.h"
#include "kresourcelist.h"
#include "vfolder_menu.h"

#include <config.h>

#include <kservice.h>
#include <kmimetype.h>
#include "kbuildservicetypefactory.h"
#include "kbuildmimetypefactory.h"
#include "kbuildservicefactory.h"
#include "kbuildservicegroupfactory.h"
#include "kbuildprotocolinfofactory.h"
#include "kctimefactory.h"
#include <ktemporaryfile.h>
#include <QtCore/QDataStream>
#include <QtCore/QDir>
#include <QtCore/QEventLoop>
#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtDBus/QtDBus>
#include <errno.h>

#include <assert.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <kstandarddirs.h>
#include <ksavefile.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kcrash.h>
#include <kmemfile.h>

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <memory> // auto_ptr

typedef QHash<QString, KSycocaEntry::Ptr> KBSEntryDict;
typedef QList<KSycocaEntry::List> KSycocaEntryListList;

static quint32 newTimestamp = 0;

static KBuildServiceFactory *g_bsf = 0;
static KBuildServiceGroupFactory *g_bsgf = 0;
static KSycocaFactory *g_factory = 0;
static KCTimeInfo *g_ctimeInfo = 0;
static QHash<QString, quint32> *g_ctimeDict = 0;
static QByteArray g_resource = 0;
static KBSEntryDict *g_entryDict = 0;
static KBSEntryDict *g_serviceGroupEntryDict = 0;
static KSycocaEntryListList *g_allEntries = 0;
static QStringList *g_changeList = 0;
static QStringList *g_allResourceDirs = 0;
static bool g_changed = false;
static KSycocaEntry::List g_tempStorage;
static VFolderMenu *g_vfolder = 0;

static const char *cSycocaPath = 0;

static bool bGlobalDatabase = false;
static bool bMenuTest = false;

void crashHandler(int)
{
   // If we crash while reading sycoca, we delete the database
   // in an attempt to recover.
   if (cSycocaPath)
      unlink(cSycocaPath);
}

static QString sycocaPath()
{
  return KSycoca::absoluteFilePath(bGlobalDatabase ? KSycoca::GlobalDatabase : KSycoca::LocalDatabase);
}

KBuildSycoca::KBuildSycoca()
  : KSycoca( true )
{
}

KBuildSycoca::~KBuildSycoca()
{

}

KSycocaEntry::Ptr KBuildSycoca::createEntry(const QString &file, bool addToFactory)
{
   quint32 timeStamp = g_ctimeInfo->ctime(file);
   if (!timeStamp)
   {
      timeStamp = KGlobal::dirs()->calcResourceHash( g_resource, file,
                                                     KStandardDirs::Recursive);
   }
   KSycocaEntry::Ptr entry;
   if (g_allEntries)
   {
      assert(g_ctimeDict);
      quint32 oldTimestamp = g_ctimeDict->value( file, 0 );

      if (timeStamp && (timeStamp == oldTimestamp))
      {
         // Re-use old entry
         if (g_factory == g_bsgf) // Strip .directory from service-group entries
         {
            entry = g_entryDict->value(file.left(file.length()-10));
         } else {
            entry = g_entryDict->value(file);
         }
         // remove from g_ctimeDict; if g_ctimeDict is not empty
         // after all files have been processed, it means
         // some files were removed since last time
         g_ctimeDict->remove( file );
      }
      else if (oldTimestamp)
      {
         g_changed = true;
         kDebug(7021) << "modified:" << file;
      }
      else
      {
         g_changed = true;
         kDebug(7021) << "new:" << file;
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
      if (addToFactory)
         g_factory->addEntry(entry);
      else
         g_tempStorage.append(entry);
      return entry;
   }
   return KSycocaEntry::Ptr();
}

KService::Ptr KBuildSycoca::createService(const QString &path)
{
   KSycocaEntry::Ptr entry = createEntry(path, false);
   return KService::Ptr::staticCast(entry);
}

// returns false if the database is up to date
bool KBuildSycoca::build()
{
  typedef QLinkedList<KBSEntryDict *> KBSEntryDictList;
  KBSEntryDictList entryDictList;
  KBSEntryDict *serviceEntryDict = 0;

  // Convert for each factory the entryList to a Dict.
  int i = 0;
  // For each factory
  for (KSycocaFactoryList::Iterator factory = factories()->begin();
       factory != factories()->end();
       ++factory)
  {
     KBSEntryDict *entryDict = new KBSEntryDict;
     if (g_allEntries)
     {
         const KSycocaEntry::List list = (*g_allEntries)[i++];
         for( KSycocaEntry::List::const_iterator it = list.begin();
            it != list.end();
            ++it)
         {
            entryDict->insert( (*it)->entryPath(), *it );
         }
     }
     if ((*factory) == g_bsf)
        serviceEntryDict = entryDict;
     else if ((*factory) == g_bsgf)
        g_serviceGroupEntryDict = entryDict;
     entryDictList.append(entryDict);
  }

  QStringList allResources; // we could use QSet<QString> - does order matter?
  // For each factory
  for (KSycocaFactoryList::Iterator factory = factories()->begin();
       factory != factories()->end();
       ++factory)
  {
    // For each resource the factory deals with
    const KSycocaResourceList *list = (*factory)->resourceList();
    if (!list) continue;

    for( KSycocaResourceList::ConstIterator it1 = list->constBegin();
         it1 != list->constEnd();
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
  for( QStringList::ConstIterator it1 = allResources.constBegin();
       it1 != allResources.constEnd();
       ++it1 )
  {
     g_changed = false;
     g_resource = (*it1).toLatin1();

     QStringList relFiles;

     (void) KGlobal::dirs()->findAllResources( g_resource,
                                               QString(),
                                               KStandardDirs::Recursive |
                                               KStandardDirs::NoDuplicates,
                                               relFiles);


     // Now find all factories that use this resource....
     // For each factory
     KBSEntryDictList::const_iterator ed_it = entryDictList.begin();
     const KBSEntryDictList::const_iterator ed_end = entryDictList.end();
     KSycocaFactoryList::const_iterator it = factories()->constBegin();
     const KSycocaFactoryList::const_iterator end = factories()->constEnd();
     for ( ; it != end; ++it, ++ed_it )
     {
        g_factory = (*it);
        // g_ctimeInfo gets created after the initial loop, so it has no entryDict.
        g_entryDict = ed_it == ed_end ? 0 : *ed_it;
	// For each resource the factory deals with
        const KSycocaResourceList *list = g_factory->resourceList();
        if (!list) continue;

        for( KSycocaResourceList::ConstIterator it2 = list->constBegin();
             it2 != list->constEnd();
             ++it2 )
        {
           KSycocaResource res = (*it2);
           if (res.resource != (*it1)) continue;

           // For each file in the resource
           for( QStringList::ConstIterator it3 = relFiles.constBegin();
                it3 != relFiles.constEnd();
                ++it3 )
           {
               // Check if file matches filter
               if ((*it3).endsWith(res.extension))
                   createEntry(*it3, true);
           }
        }
     }
     if (g_changed || !g_allEntries)
     {
        uptodate = false;
        g_changeList->append(g_resource);
     }
  }

  bool result = !uptodate || (g_ctimeDict && !g_ctimeDict->isEmpty());

  if (result || bMenuTest)
  {
     g_resource = "apps";
     g_factory = g_bsf;
     g_entryDict = serviceEntryDict;
     g_changed = false;

     g_vfolder = new VFolderMenu(g_bsf, this);
     if (!m_trackId.isEmpty())
        g_vfolder->setTrackId(m_trackId);

     VFolderMenu::SubMenu *kdeMenu = g_vfolder->parseMenu("applications.menu", true);

     KServiceGroup::Ptr entry = g_bsgf->addNew("/", kdeMenu->directoryFile, KServiceGroup::Ptr(), false);
     entry->setLayoutInfo(kdeMenu->layoutList);
     createMenu(QString(), QString(), kdeMenu);

     (void) existingResourceDirs();
     *g_allResourceDirs += g_vfolder->allDirectories();

     if (g_changed || !g_allEntries)
     {
        uptodate = false;
        g_changeList->append(g_resource);
     }
     if (bMenuTest) {
         result = false;
     }
  }

  qDeleteAll(entryDictList);
  return result;
}

void KBuildSycoca::createMenu(const QString &caption_, const QString &name_, VFolderMenu::SubMenu *menu)
{
  QString caption = caption_;
  QString name = name_;
  foreach (VFolderMenu::SubMenu *subMenu, menu->subMenus)
  {
     QString subName = name+subMenu->name+'/';

     QString directoryFile = subMenu->directoryFile;
     if (directoryFile.isEmpty())
        directoryFile = subName+".directory";
     quint32 timeStamp = g_ctimeInfo->ctime(directoryFile);
     if (!timeStamp) {
        timeStamp = KGlobal::dirs()->calcResourceHash( g_resource, directoryFile,
                                                       KStandardDirs::Recursive );
     }

     KServiceGroup::Ptr entry;
     if (g_allEntries)
     {
        quint32 oldTimestamp = g_ctimeDict->value( directoryFile, 0 );

        if (timeStamp && (timeStamp == oldTimestamp))
        {
            KSycocaEntry::Ptr group = g_serviceGroupEntryDict->value(subName);
            if ( group )
            {
                entry = KServiceGroup::Ptr::staticCast( group );
                if (entry->directoryEntryPath() != directoryFile)
                    entry = 0; // Can't reuse this one!
            }
        }
     }
     g_ctimeInfo->addCTime(directoryFile, timeStamp);

     entry = g_bsgf->addNew(subName, subMenu->directoryFile, entry, subMenu->isDeleted);
     entry->setLayoutInfo(subMenu->layoutList);
     if (! (bMenuTest && entry->noDisplay()) )
        createMenu(caption + entry->caption() + '/', subName, subMenu);
  }
  if (caption.isEmpty())
     caption += '/';
  if (name.isEmpty())
     name += '/';
  foreach (const KService::Ptr &p, menu->items)
  {
     if (bMenuTest)
     {
        if (!menu->isDeleted && !p->noDisplay())
           printf("%s\t%s\t%s\n", qPrintable( caption ), qPrintable( p->menuId() ), qPrintable( KStandardDirs::locate("apps", p->entryPath() ) ) );
     }
     else
     {
        g_bsgf->addNewEntryTo( name, p );
     }
  }
}

bool KBuildSycoca::recreate()
{
  QString path(sycocaPath());

  // KSaveFile first writes to a temp file.
  // Upon finalize() it moves the stuff to the right place.
  KSaveFile database(path);
  bool openedOK = database.open();
  if (!openedOK && database.error() == QFile::PermissionsError && QFile::exists(path))
  {
    QFile::remove( path );
    openedOK = database.open();
  }
  if (!openedOK)
  {
    fprintf(stderr, "kbuildsycoca4: ERROR creating database '%s'! %s\n",
      path.toLocal8Bit().data(), database.errorString().toLocal8Bit().data());
    return false;
  }

  QDataStream* str = new QDataStream(&database);
  str->setVersion(QDataStream::Qt_3_1);

  kDebug(7021).nospace() << "Recreating ksycoca file (" << path << ", version " << KSycoca::version() << ")";

  // It is very important to build the servicetype one first
  // Both are registered in KSycoca, no need to keep the pointers
  KSycocaFactory *stf = new KBuildServiceTypeFactory;
  KBuildMimeTypeFactory *mtf = new KBuildMimeTypeFactory;
  g_bsgf = new KBuildServiceGroupFactory();
  g_bsf = new KBuildServiceFactory(stf, mtf, g_bsgf);
  (void) new KBuildProtocolInfoFactory();

  if( build()) // Parse dirs
  {
    save(str); // Save database
    if (str->status() != QDataStream::Ok) // ######## TODO: does this detect write errors, e.g. disk full?
      database.abort(); // Error
    delete str;
    str = 0;
    if (!database.finalize())
    {
      fprintf(stderr, "kbuildsycoca4: ERROR writing database '%s'!\n", database.fileName().toLocal8Bit().data());
      fprintf(stderr, "kbuildsycoca4: Disk full?\n");
      return false;
    }
  }
  else
  {
    delete str;
    str = 0;
    database.abort();
    if (bMenuTest)
       return true;
    kDebug(7021) << "Database is up to date";
  }

  if (!bGlobalDatabase)
  {
    // update the timestamp file
    QString stamppath = path + "stamp";
    QFile ksycocastamp(stamppath);
    ksycocastamp.open( QIODevice::WriteOnly );
    QDataStream str( &ksycocastamp );
    str.setVersion(QDataStream::Qt_3_1);
    str << newTimestamp;
    str << existingResourceDirs();
    if (g_vfolder)
        str << g_vfolder->allDirectories(); // Extra resource dirs
  }
  if (d->m_sycocaStrategy == KSycocaPrivate::StrategyMemFile)
     KMemFile::fileContentsChanged(path);

  return true;
}

void KBuildSycoca::save(QDataStream* str)
{
   // Write header (#pass 1)
   str->device()->seek(0);

   (*str) << (qint32) KSycoca::version();
   KSycocaFactory * servicetypeFactory = 0;
   KBuildMimeTypeFactory * mimeTypeFactory = 0;
   KBuildServiceFactory * serviceFactory = 0;
   for(KSycocaFactoryList::Iterator factory = factories()->begin();
       factory != factories()->end();
       ++factory)
   {
      qint32 aId;
      qint32 aOffset;
      aId = (*factory)->factoryId();
      if ( aId == KST_KServiceTypeFactory )
         servicetypeFactory = *factory;
      else if ( aId == KST_KMimeTypeFactory )
         mimeTypeFactory = static_cast<KBuildMimeTypeFactory *>( *factory );
      else if ( aId == KST_KServiceFactory )
         serviceFactory = static_cast<KBuildServiceFactory *>( *factory );
      aOffset = (*factory)->offset(); // not set yet, so always 0
      (*str) << aId;
      (*str) << aOffset;
   }
   (*str) << (qint32) 0; // No more factories.
   // Write KDEDIRS
   (*str) << KGlobal::dirs()->kfsstnd_prefixes();
   (*str) << newTimestamp;
   (*str) << KGlobal::locale()->language();
   (*str) << KGlobal::dirs()->calcResourceHash("services", "update_ksycoca",
                                                 KStandardDirs::Recursive );
   (*str) << (*g_allResourceDirs);

   // Calculate per-servicetype/mimetype data
   serviceFactory->postProcessServices();

   // Here so that it's the last debug message
   kDebug(7021) << "Saving";

   // Write factory data....
   for(KSycocaFactoryList::Iterator factory = factories()->begin();
       factory != factories()->end();
       ++factory)
   {
      (*factory)->save(*str);
      if (str->status() != QDataStream::Ok) // ######## TODO: does this detect write errors, e.g. disk full?
         return; // error
   }

   int endOfData = str->device()->pos();

   // Write header (#pass 2)
   str->device()->seek(0);

   (*str) << (qint32) KSycoca::version();
   for(KSycocaFactoryList::Iterator factory = factories()->begin();
       factory != factories()->end(); ++factory)
   {
      qint32 aId;
      qint32 aOffset;
      aId = (*factory)->factoryId();
      aOffset = (*factory)->offset();
      (*str) << aId;
      (*str) << aOffset;
   }
   (*str) << (qint32) 0; // No more factories.

   // Jump to end of database
   str->device()->seek(endOfData);
}

bool KBuildSycoca::checkDirTimestamps( const QString& dirname, const QDateTime& stamp, bool top )
{
   if( top )
   {
      QFileInfo inf( dirname );
      if( inf.lastModified() > stamp ) {
         kDebug( 7021 ) << "timestamp changed:" << dirname;
         return false;
      }
   }
   QDir dir( dirname );
   const QFileInfoList list = dir.entryInfoList( QDir::NoFilter, QDir::Unsorted );
   if (list.isEmpty())
      return true;

   foreach ( const QFileInfo& fi, list ) {
      if( fi.fileName() == "." || fi.fileName() == ".." )
         continue;
      if( fi.lastModified() > stamp )
      {
         kDebug( 7201 ) << "timestamp changed:" << fi.filePath();
         return false;
      }
      if( fi.isDir() && !checkDirTimestamps( fi.filePath(), stamp, false ))
            return false;
   }
   return true;
}

// check times of last modification of all files on which ksycoca depens,
// and also their directories
// if all of them are older than the timestamp in file ksycocastamp, this
// means that there's no need to rebuild ksycoca
bool KBuildSycoca::checkTimestamps( quint32 timestamp, const QStringList &dirs )
{
   kDebug( 7021 ) << "checking file timestamps";
   QDateTime stamp;
   stamp.setTime_t( timestamp );
   for( QStringList::ConstIterator it = dirs.begin();
        it != dirs.end();
        ++it )
   {
      if( !checkDirTimestamps( *it, stamp, true ))
            return false;
   }
   kDebug( 7021 ) << "timestamps check ok";
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
   resources += KBuildMimeTypeFactory::resourceTypes();
   resources += KBuildServiceGroupFactory::resourceTypes();
   resources += KBuildServiceFactory::resourceTypes();
   resources += KBuildProtocolInfoFactory::resourceTypes();
   while( !resources.empty())
   {
      QString res = resources.front();
      *dirs += KGlobal::dirs()->resourceDirs( res.toLatin1());
      resources.removeAll( res );
   }

   *g_allResourceDirs = *dirs;

   for( QStringList::Iterator it = dirs->begin();
        it != dirs->end(); )
   {
      QFileInfo inf( *it );
      if( !inf.exists() || !inf.isReadable() )
         it = dirs->erase( it );
      else
         ++it;
   }
   return *dirs;
}

static const char appFullName[] = "org.kde.kbuildsycoca";
static const char appName[] = "kbuildsycoca4";
static const char appVersion[] = "1.1";

extern "C" KDE_EXPORT int kdemain(int argc, char **argv)
{
   KAboutData d(appName, "kdelibs4", ki18n("KBuildSycoca"), appVersion,
                ki18n("Rebuilds the system configuration cache."),
                KAboutData::License_GPL, ki18n("(c) 1999-2002 KDE Developers"));
   d.addAuthor(ki18n("David Faure"), ki18n("Author"), "faure@kde.org");
   d.addAuthor(ki18n("Waldo Bastian"), ki18n("Author"), "bastian@kde.org");

   KCmdLineOptions options;
   options.add("nosignal", ki18n("Do not signal applications to update"));
   options.add("noincremental", ki18n("Disable incremental update, re-read everything"));
   options.add("checkstamps", ki18n("Check file timestamps"));
   options.add("nocheckfiles", ki18n("Disable checking files (dangerous)"));
   options.add("global", ki18n("Create global database"));
   options.add("menutest", ki18n("Perform menu generation test run only"));
   options.add("track <menu-id>", ki18n("Track menu id for debug purposes"));

   KCmdLineArgs::init(argc, argv, &d);
   KCmdLineArgs::addCmdLineOptions(options);
   KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
   bGlobalDatabase = args->isSet("global");
   bMenuTest = args->isSet("menutest");

   if (bGlobalDatabase)
   {
     setenv("KDEHOME", "-", 1);
     setenv("KDEROOTHOME", "-", 1);
   }

   QCoreApplication k(argc, argv);
   KComponentData mainComponent(d);

   KCrash::setCrashHandler(KCrash::defaultCrashHandler);
   KCrash::setEmergencySaveFunction(crashHandler);
   KCrash::setApplicationName(QString(appName));

   // force generating of KLocale object. if not, the database will get
   // be translated
   KGlobal::locale();
   KGlobal::dirs()->addResourceType("app-reg", 0, "share/application-registry" );

   while(QDBusConnection::sessionBus().isConnected())
   {
     // kapp registered already, but with the PID in the name.
     // We need to re-register without it, to detect already-running kbuildsycoca instances.
     if (QDBusConnection::sessionBus().interface()->registerService(appFullName, QDBusConnectionInterface::QueueService)
         != QDBusConnectionInterface::ServiceQueued)
     {
       break; // Go
     }
     fprintf(stderr, "Waiting for already running %s to finish.\n", appName);

     QEventLoop eventLoop;
     QObject::connect(QDBusConnection::sessionBus().interface(), SIGNAL(serviceRegistered(QString)),
                      &eventLoop, SLOT(quit()));
     eventLoop.exec( QEventLoop::ExcludeUserInputEvents );
   }
   fprintf(stderr, "%s running...\n", appName);

   bool checkfiles = bGlobalDatabase || args->isSet("checkfiles");

   bool incremental = !bGlobalDatabase && args->isSet("incremental") && checkfiles;
   if (incremental || !checkfiles)
   {
     KSycoca::disableAutoRebuild(); // Prevent deadlock
     QString current_language = KGlobal::locale()->language();
     QString ksycoca_language = KSycoca::self()->language();
     quint32 current_update_sig = KGlobal::dirs()->calcResourceHash("services", "update_ksycoca",
                                                                    KStandardDirs::Recursive );
     quint32 ksycoca_update_sig = KSycoca::self()->updateSignature();
     QString current_prefixes = KGlobal::dirs()->kfsstnd_prefixes();
     QString ksycoca_prefixes = KSycoca::self()->kfsstnd_prefixes();

     if ((current_update_sig != ksycoca_update_sig) ||
         (current_language != ksycoca_language) ||
         (current_prefixes != ksycoca_prefixes) ||
         (KSycoca::self()->timeStamp() == 0))
     {
        incremental = false;
        checkfiles = true;
        KBuildSycoca::clearCaches();
     }
   }

   g_changeList = new QStringList;

   bool checkstamps = incremental && args->isSet("checkstamps") && checkfiles;
   quint32 filestamp = 0;
   QStringList oldresourcedirs;
   if( checkstamps && incremental )
   {
       QString path = sycocaPath()+"stamp";
       QByteArray qPath = QFile::encodeName(path);
       cSycocaPath = qPath.data(); // Delete timestamps on crash
       QFile ksycocastamp(path);
       if( ksycocastamp.open( QIODevice::ReadOnly ))
       {
           QDataStream str( &ksycocastamp );
           str.setVersion(QDataStream::Qt_3_1);

           if (!str.atEnd())
               str >> filestamp;
           if (!str.atEnd())
           {
               str >> oldresourcedirs;
               if( oldresourcedirs != KBuildSycoca::existingResourceDirs())
                   checkstamps = false;
           }
           else
           {
               checkstamps = false;
           }
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
       cSycocaPath = 0;
   }

   newTimestamp = (quint32) time(0);

   if( checkfiles && ( !checkstamps || !KBuildSycoca::checkTimestamps( filestamp, oldresourcedirs )))
   {
      QByteArray qSycocaPath = QFile::encodeName(sycocaPath());
      cSycocaPath = qSycocaPath.data();

      g_allEntries = 0;
      g_ctimeDict = 0;
      if (incremental)
      {
         kDebug(7021) << "Reusing existing ksycoca";
         KSycoca::self();
         KSycocaFactoryList *factories = new KSycocaFactoryList;
         g_allEntries = new KSycocaEntryListList;
         g_ctimeDict = new QHash<QString, quint32>;

         // Must be in same order as in KBuildSycoca::recreate()!
         factories->append( new KServiceTypeFactory );
         factories->append( new KMimeTypeFactory );
         factories->append( new KServiceGroupFactory );
         factories->append( new KServiceFactory );
         factories->append( new KProtocolInfoFactory );

         // For each factory
	 for (KSycocaFactoryList::Iterator factory = factories->begin();
	      factory != factories->end(); ++factory)
         {
             const KSycocaEntry::List list = (*factory)->allEntries();
             g_allEntries->append( list );
         }
         delete factories; factories = 0;
         KCTimeInfo *ctimeInfo = new KCTimeInfo;
         ctimeInfo->fillCTimeDict(*g_ctimeDict);
      }
      cSycocaPath = 0;

      KBuildSycoca *sycoca = new KBuildSycoca; // Build data base (deletes oldSycoca)
      if (args->isSet("track"))
         sycoca->setTrackId(args->getOption("track"));
      if (!sycoca->recreate()) {
        return -1;
      }

      if (bGlobalDatabase)
      {
        // These directories may have been created with 0700 permission
        // better delete them if they are empty
        QString applnkDir = KGlobal::dirs()->saveLocation("apps", QString(), false);
        ::rmdir(QFile::encodeName(applnkDir));
        QString servicetypesDir = KGlobal::dirs()->saveLocation("servicetypes", QString(), false);
        ::rmdir(QFile::encodeName(servicetypesDir));
      }
   }

   if (args->isSet("signal"))
   {
     // Notify ALL applications that have a ksycoca object, using a signal
     QDBusMessage signal = QDBusMessage::createSignal("/", "org.kde.KSycoca", "notifyDatabaseChanged" );
     signal << *g_changeList;

     if (QDBusConnection::sessionBus().isConnected()) {
        kDebug() << "Emitting notifyDatabaseChanged" << *g_changeList;
       QDBusConnection::sessionBus().send(signal);
       qApp->processEvents(); // make sure the dbus signal is sent before we quit.
     }
   }

   return 0;
}

#include "kbuildsycoca.moc"
