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
#include "ksycocaresourcelist.h"
#include "vfolder_menu.h"

#include <kservice.h>
#include "kbuildservicetypefactory.h"
#include "kbuildmimetypefactory.h"
#include "kbuildservicefactory.h"
#include "kbuildservicegroupfactory.h"
#include "kctimefactory.h"
#include <QtCore/QDataStream>
#include <QtCore/QDir>
#include <QtCore/QEventLoop>
#include <QtCore/QFile>
#include <QtCore/QLocale>
#include <QtCore/QTimer>
#include <QtCore/QDebug>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusConnectionInterface>
#include <QDirIterator>
#include <QDateTime>
#include <qsavefile.h>
#include <errno.h>

#include <assert.h>

#include <kcrash.h>
#include <kmemfile_p.h>
#include <kaboutdata.h>

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <memory> // auto_ptr
#include <qstandardpaths.h>
#include <qcommandlineparser.h>
#include <qcommandlineoption.h>

typedef QHash<QString, KSycocaEntry::Ptr> KBSEntryDict;
typedef QList<KSycocaEntry::List> KSycocaEntryListList;

static quint32 newTimestamp = 0;

static KBuildServiceFactory *g_serviceFactory = 0;
static KBuildServiceGroupFactory *g_buildServiceGroupFactory = 0;
static KSycocaFactory *g_currentFactory = 0;
static KCTimeInfo *g_ctimeInfo = 0; // factory
static KCTimeDict *g_ctimeDict = 0; // old timestamps
static KBSEntryDict *g_currentEntryDict = 0;
static KBSEntryDict *g_serviceGroupEntryDict = 0;
static KSycocaEntryListList *g_allEntries = 0; // entries from existing ksycoca
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
  QFileInfo fi(KSycoca::absoluteFilePath(bGlobalDatabase ? KSycoca::GlobalDatabase : KSycoca::LocalDatabase));
  if (!QDir().mkpath(fi.absolutePath())) {
      qWarning() << "Couldn't create" << fi.absolutePath();
  }
  return fi.absoluteFilePath();
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
   quint32 timeStamp = g_ctimeInfo->dict()->ctime(file, g_resource);
   if (!timeStamp)
   {
      timeStamp = calcResourceHash(g_resourceSubdir, file);
   }
   KSycocaEntry::Ptr entry;
   if (g_allEntries)
   {
      assert(g_ctimeDict);
      quint32 oldTimestamp = g_ctimeDict->ctime(file, g_resource);
      if (file.contains("fake"))
          qDebug() << "g_ctimeDict->ctime(" << file << ") = " << oldTimestamp << "compared with" << timeStamp;

      if (timeStamp && (timeStamp == oldTimestamp))
      {
         // Re-use old entry
         if (g_currentFactory == g_buildServiceGroupFactory) // Strip .directory from service-group entries
         {
            entry = g_currentEntryDict->value(file.left(file.length()-10));
         } else {
            entry = g_currentEntryDict->value(file);
         }
         // remove from g_ctimeDict; if g_ctimeDict is not empty
         // after all files have been processed, it means
         // some files were removed since last time
         if (file.contains("fake"))
             qDebug() << "reusing (and removing) old entry for:" << file << "entry=" << entry;
         g_ctimeDict->remove(file, g_resource);
      }
      else if (oldTimestamp)
      {
         g_changed = true;
         g_ctimeDict->remove(file, g_resource);
         qDebug() << "modified:" << file;
      }
      else
      {
         g_changed = true;
         qDebug() << "new:" << file;
      }
   }
   g_ctimeInfo->dict()->addCTime(file, g_resource, timeStamp);
   if (!entry)
   {
      // Create a new entry
      entry = g_currentFactory->createEntry(file);
   }
   if ( entry && entry->isValid() )
   {
      if (addToFactory)
         g_currentFactory->addEntry(entry);
      else
         g_tempStorage.append(entry);
      return entry;
   }
   return KSycocaEntry::Ptr();
}

KService::Ptr KBuildSycoca::createService(const QString &path)
{
   KSycocaEntry::Ptr entry = createEntry(path, false);
   return KService::Ptr(entry);
}

// returns false if the database is up to date, true if it needs to be saved
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
         Q_FOREACH(const KSycocaEntry::Ptr& entry, list) {
             //if (entry->entryPath().contains("fake"))
             //    qDebug() << "inserting into entryDict:" << entry->entryPath() << entry;
             entryDict->insert(entry->entryPath(), entry);
         }
     }
     if ((*factory) == g_serviceFactory)
        serviceEntryDict = entryDict;
     else if ((*factory) == g_buildServiceGroupFactory)
        g_serviceGroupEntryDict = entryDict;
     entryDictList.append(entryDict);
  }

  QMap<QString, QByteArray> allResourcesSubDirs; // dirs, kstandarddirs-resource-name
  // For each factory
  for (KSycocaFactoryList::Iterator factory = factories()->begin();
       factory != factories()->end();
       ++factory)
  {
    // For each resource the factory deals with
    const KSycocaResourceList *list = (*factory)->resourceList();
    if (!list) continue;
    Q_FOREACH (const KSycocaResource& res, *list) {
       // With this we would get dirs, but not a unique list of relative files (for global+local merging to work)
       //const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, res.subdir, QStandardPaths::LocateDirectory);
       //allResourcesSubDirs[res.resource] += dirs;
       allResourcesSubDirs.insert(res.subdir, res.resource);
    }
  }

  g_ctimeInfo = new KCTimeInfo(); // This is a build factory too, don't delete!!
  bool uptodate = true;
  for( QMap<QString, QByteArray>::ConstIterator it1 = allResourcesSubDirs.constBegin();
       it1 != allResourcesSubDirs.constEnd();
       ++it1 )
  {
     g_changed = false;
     g_resourceSubdir = it1.key();
     g_resource = it1.value();

     QStringList relFiles;
     const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, g_resourceSubdir, QStandardPaths::LocateDirectory);
     Q_FOREACH(const QString& dir, dirs) {
         QDirIterator it(dir, QDirIterator::Subdirectories);
         while (it.hasNext()) {
             const QString filePath = it.next();
             Q_ASSERT(filePath.startsWith(dir)); // due to the line below...
             const QString relPath = filePath.mid(dir.length()+1);
             if (!relFiles.contains(relPath))
                 relFiles.append(relPath);
         }
     }
     // Now find all factories that use this resource....
     // For each factory
     KBSEntryDictList::const_iterator ed_it = entryDictList.begin();
     const KBSEntryDictList::const_iterator ed_end = entryDictList.end();
     KSycocaFactoryList::const_iterator it = factories()->constBegin();
     const KSycocaFactoryList::const_iterator end = factories()->constEnd();
     for ( ; it != end; ++it, ++ed_it )
     {
        g_currentFactory = (*it);
        // g_ctimeInfo gets created after the initial loop, so it has no entryDict.
        g_currentEntryDict = ed_it == ed_end ? 0 : *ed_it;
	// For each resource the factory deals with
        const KSycocaResourceList *list = g_currentFactory->resourceList();
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
              if ((*it3).endsWith(res.extension)) {
                 QString entryPath = (*it3);
                 createEntry(entryPath, true);
              }
           }
        }
     }
     if (g_changed || !g_allEntries)
     {
        uptodate = false;
        //qDebug() << "CHANGED:" << resource;
        m_changedResources.append(g_resource);
     }
  }

  bool result = !uptodate || (g_ctimeDict && !g_ctimeDict->isEmpty());
  if (g_ctimeDict && !g_ctimeDict->isEmpty()) {
      //qDebug() << "Still in time dict:";
      //g_ctimeDict->dump();
      // ## It seems entries filtered out by vfolder are still in there,
      // so we end up always saving ksycoca, i.e. this method never returns false

      // Get the list of resources from which some files were deleted
      const QStringList resources = g_ctimeDict->remainingResourceList();
      qDebug() << "Still in the time dict (i.e. deleted files)" << resources;
      m_changedResources += resources;
  }

  if (result || bMenuTest)
  {
     g_resource = "apps";
     g_resourceSubdir = "applications";
     g_currentFactory = g_serviceFactory;
     g_currentEntryDict = serviceEntryDict;
     g_changed = false;

     g_vfolder = new VFolderMenu(g_serviceFactory, this);
     if (!m_trackId.isEmpty())
        g_vfolder->setTrackId(m_trackId);

     VFolderMenu::SubMenu *kdeMenu = g_vfolder->parseMenu("applications.menu");

     KServiceGroup::Ptr entry = g_buildServiceGroupFactory->addNew("/", kdeMenu->directoryFile, KServiceGroup::Ptr(), false);
     entry->setLayoutInfo(kdeMenu->layoutList);
     createMenu(QString(), QString(), kdeMenu);

     (void) existingResourceDirs();
     *g_allResourceDirs += g_vfolder->allDirectories();

     if (g_changed || !g_allEntries)
     {
        uptodate = false;
        //qDebug() << "CHANGED:" << g_resource;
        m_changedResources.append(g_resource);
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
     quint32 timeStamp = g_ctimeInfo->dict()->ctime(directoryFile, g_resource);
     if (!timeStamp) {
        timeStamp = calcResourceHash(g_resourceSubdir, directoryFile);
     }

     KServiceGroup::Ptr entry;
     if (g_allEntries)
     {
        const quint32 oldTimestamp = g_ctimeDict->ctime(directoryFile, g_resource);

        if (timeStamp && (timeStamp == oldTimestamp))
        {
            KSycocaEntry::Ptr group = g_serviceGroupEntryDict->value(subName);
            if ( group )
            {
                entry = KServiceGroup::Ptr( group );
                if (entry->directoryEntryPath() != directoryFile)
                    entry = 0; // Can't reuse this one!
            }
        }
     }
     if (timeStamp) // bug? (see calcResourceHash). There might not be a .directory file...
         g_ctimeInfo->dict()->addCTime(directoryFile, g_resource, timeStamp);

     entry = g_buildServiceGroupFactory->addNew(subName, subMenu->directoryFile, entry, subMenu->isDeleted);
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
            printf("%s\t%s\t%s\n", qPrintable( caption ), qPrintable( p->menuId() ),
                   qPrintable(QStandardPaths::locate(QStandardPaths::ApplicationsLocation, p->entryPath())));
     }
     else
     {
        g_buildServiceGroupFactory->addNewEntryTo( name, p );
     }
  }
}

bool KBuildSycoca::recreate()
{
  QString path(sycocaPath());

  QSaveFile database(path);
  bool openedOK = database.open(QIODevice::WriteOnly);
  if (!openedOK && database.error() == QFile::PermissionsError && QFile::exists(path))
  {
    QFile::remove( path );
    openedOK = database.open(QIODevice::WriteOnly);
  }
  if (!openedOK)
  {
    fprintf(stderr, KBUILDSYCOCA_EXENAME ": ERROR creating database '%s'! %s\n",
      path.toLocal8Bit().data(), database.errorString().toLocal8Bit().data());
    return false;
  }

  QDataStream* str = new QDataStream(&database);
  str->setVersion(QDataStream::Qt_3_1);

  qDebug().nospace() << "Recreating ksycoca file (" << path << ", version " << KSycoca::version() << ")";

  // It is very important to build the servicetype one first
  // Both are registered in KSycoca, no need to keep the pointers
  KSycocaFactory *stf = new KBuildServiceTypeFactory;
  KBuildMimeTypeFactory* mimeTypeFactory = new KBuildMimeTypeFactory;
  g_buildServiceGroupFactory = new KBuildServiceGroupFactory();
  g_serviceFactory = new KBuildServiceFactory(stf, mimeTypeFactory, g_buildServiceGroupFactory);

  if (build()) // Parse dirs
  {
    save(str); // Save database
    if (str->status() != QDataStream::Ok) // Probably unnecessary now in Qt5, since QSaveFile detects write errors
      database.cancelWriting(); // Error
    delete str;
    str = 0;
    if (!database.commit())
    {
      fprintf(stderr, KBUILDSYCOCA_EXENAME ": ERROR writing database '%s'!\n", database.fileName().toLocal8Bit().data());
      fprintf(stderr, KBUILDSYCOCA_EXENAME ": Disk full?\n");
      return false;
    }
  }
  else
  {
    delete str;
    str = 0;
    database.cancelWriting();
    if (bMenuTest)
       return true;
    qDebug() << "Database is up to date";
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
   //KSycocaFactory * servicetypeFactory = 0;
   //KBuildMimeTypeFactory * mimeTypeFactory = 0;
   KBuildServiceFactory * serviceFactory = 0;
   for(KSycocaFactoryList::Iterator factory = factories()->begin();
       factory != factories()->end();
       ++factory)
   {
      qint32 aId;
      qint32 aOffset;
      aId = (*factory)->factoryId();
      //if ( aId == KST_KServiceTypeFactory )
      //   servicetypeFactory = *factory;
      //else if ( aId == KST_KMimeTypeFactory )
      //   mimeTypeFactory = static_cast<KBuildMimeTypeFactory *>( *factory );
      if ( aId == KST_KServiceFactory )
         serviceFactory = static_cast<KBuildServiceFactory *>( *factory );
      aOffset = (*factory)->offset(); // not set yet, so always 0
      (*str) << aId;
      (*str) << aOffset;
   }
   (*str) << (qint32) 0; // No more factories.
   // Write XDG_DATA_DIRS
   (*str) << QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).join(QString(QLatin1Char(':')));
   (*str) << newTimestamp;
   (*str) << QLocale::languageToString(QLocale().language());
   // This makes it possible to trigger a ksycoca update for all users (KIOSK feature)
   (*str) << calcResourceHash("kde5/services", "update_ksycoca");
   (*str) << (*g_allResourceDirs);

   // Calculate per-servicetype/mimetype data
   serviceFactory->postProcessServices();

   // Here so that it's the last debug message
   qDebug() << "Saving";

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
         qDebug() << "timestamp changed:" << dirname;
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
         qDebug() << "timestamp changed:" << fi.filePath();
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
   qDebug() << "checking file timestamps";
   QDateTime stamp;
   stamp.setTime_t( timestamp );
   for( QStringList::ConstIterator it = dirs.begin();
        it != dirs.end();
        ++it )
   {
      if( !checkDirTimestamps( *it, stamp, true ))
            return false;
   }
   qDebug() << "timestamps check ok";
   return true;
}

QStringList KBuildSycoca::existingResourceDirs()
{
   static QStringList* dirs = NULL;
   if( dirs != NULL )
       return *dirs;
   dirs = new QStringList;
   g_allResourceDirs = new QStringList;
   // these are all resource dirs cached by ksycoca
   *dirs += KBuildServiceTypeFactory::resourceDirs();
   *dirs += KBuildMimeTypeFactory::resourceDirs();
   *dirs += KBuildServiceGroupFactory::resourceDirs();
   *dirs += KBuildServiceFactory::resourceDirs();

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
static const char appVersion[] = "5.0";

int main(int argc, char **argv)
{
   QCoreApplication app(argc, argv);
   app.setApplicationVersion(appVersion);
   KAboutData about(KBUILDSYCOCA_EXENAME, "kdelibs4", QCoreApplication::translate("main", "KBuildSycoca"), appVersion,
                QCoreApplication::translate("main", "Rebuilds the system configuration cache."),
                KAboutData::License_GPL, QCoreApplication::translate("main", "(c) 1999-2002 KDE Developers"));
   about.addAuthor(QCoreApplication::translate("main", "David Faure"), QCoreApplication::translate("main", "Author"), "faure@kde.org");
   about.addAuthor(QCoreApplication::translate("main", "Waldo Bastian"), QCoreApplication::translate("main", "Author"), "bastian@kde.org");


   QCommandLineParser parser;
   about.setupCommandLine(&parser);
   parser.addVersionOption();
   parser.setApplicationDescription(QCoreApplication::translate("main", "Rebuilds the system configuration cache."));
   parser.addHelpOption();
   parser.addOption(QCommandLineOption(QStringList() << "nosignal", QCoreApplication::translate("main", "Do not signal applications to update")));
   parser.addOption(QCommandLineOption(QStringList() << "noincremental", QCoreApplication::translate("main", "Disable incremental update, re-read everything")));
   parser.addOption(QCommandLineOption(QStringList() << "checkstamps", QCoreApplication::translate("main", "Check file timestamps")));
   parser.addOption(QCommandLineOption(QStringList() << "nocheckfiles", QCoreApplication::translate("main", "Disable checking files (dangerous)")));
   parser.addOption(QCommandLineOption(QStringList() << "global", QCoreApplication::translate("main", "Create global database")));
   parser.addOption(QCommandLineOption(QStringList() << "menutest", QCoreApplication::translate("main", "Perform menu generation test run only")));
   parser.addOption(QCommandLineOption(QStringList() << "track", QCoreApplication::translate("main", "Track menu id for debug purposes"), "menu-id"));
   parser.addOption(QCommandLineOption(QStringList() << "testmode", QCoreApplication::translate("main", "Switch QStandardPaths to test mode, for unit tests only")));
   parser.process(app);
   about.processCommandLine(&parser);

   bGlobalDatabase = parser.isSet("global");
   bMenuTest = parser.isSet("menutest");

   if (parser.isSet("testmode")) {
       QStandardPaths::enableTestMode(true);
   }

   if (bGlobalDatabase)
   {
     qputenv("XDG_DATA_HOME", "-");
   }

    KCrash::setCrashHandler(KCrash::defaultCrashHandler);
    KCrash::setEmergencySaveFunction(crashHandler);
    KCrash::setApplicationName(QString::fromLatin1(KBUILDSYCOCA_EXENAME));

   // force generating of KLocale object. if not, the database will get
   // be translated
   QLocale::setDefault(QLocale::C);

   while(QDBusConnection::sessionBus().isConnected())
   {
     // kapp registered already, but with the PID in the name.
     // We need to re-register without it, to detect already-running kbuildsycoca instances.
     if (QDBusConnection::sessionBus().interface()->registerService(appFullName, QDBusConnectionInterface::QueueService)
         != QDBusConnectionInterface::ServiceQueued)
     {
       break; // Go
     }
     fprintf(stderr, "Waiting for already running %s to finish.\n", KBUILDSYCOCA_EXENAME);

     QEventLoop eventLoop;
     QObject::connect(QDBusConnection::sessionBus().interface(), SIGNAL(serviceRegistered(QString)),
                      &eventLoop, SLOT(quit()));
     eventLoop.exec( QEventLoop::ExcludeUserInputEvents );
   }
   fprintf(stderr, "%s running...\n", KBUILDSYCOCA_EXENAME);

   bool checkfiles = bGlobalDatabase || !parser.isSet("nocheckfiles");

   bool incremental = !bGlobalDatabase && !parser.isSet("noincremental") && checkfiles;
   if (incremental || !checkfiles)
   {
     KSycoca::disableAutoRebuild(); // Prevent deadlock
     QString current_language = QLocale::languageToString(QLocale().language());
     QString ksycoca_language = KSycoca::self()->language();
     quint32 current_update_sig = KBuildSycoca::calcResourceHash("kde5/services", "update_ksycoca");
     quint32 ksycoca_update_sig = KSycoca::self()->updateSignature();
     QString current_prefixes = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).join(QString(QLatin1Char(':')));
     QString ksycoca_prefixes = static_cast<KBuildSycoca*>(KSycoca::self())->kfsstnd_prefixes();

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

   bool checkstamps = incremental && parser.isSet("checkstamps") && checkfiles;
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
   QStringList changedResources;

   if( checkfiles && ( !checkstamps || !KBuildSycoca::checkTimestamps( filestamp, oldresourcedirs )))
   {
      QByteArray qSycocaPath = QFile::encodeName(sycocaPath());
      cSycocaPath = qSycocaPath.data();

      g_allEntries = 0;
      g_ctimeDict = 0;
      if (incremental)
      {
         qDebug() << "Reusing existing ksycoca";
         KSycoca::self();
         KSycocaFactoryList *factories = new KSycocaFactoryList;
         g_allEntries = new KSycocaEntryListList;
         g_ctimeDict = new KCTimeDict;

         // Must be in same order as in KBuildSycoca::recreate()!
         factories->append( new KServiceTypeFactory );
         factories->append( new KMimeTypeFactory );
         factories->append( new KServiceGroupFactory );
         factories->append( new KServiceFactory );

         // For each factory
	 for (KSycocaFactoryList::Iterator factory = factories->begin();
	      factory != factories->end(); ++factory)
         {
             const KSycocaEntry::List list = (*factory)->allEntries();
             g_allEntries->append( list );
         }
         delete factories; factories = 0;
         KCTimeInfo *ctimeInfo = new KCTimeInfo;
         *g_ctimeDict = ctimeInfo->loadDict();
      }
      cSycocaPath = 0;

      KBuildSycoca *sycoca = new KBuildSycoca; // Build data base (deletes oldSycoca)
      if (parser.isSet("track"))
         sycoca->setTrackId(parser.value("track"));
      if (!sycoca->recreate()) {
        return -1;
      }
      changedResources = sycoca->changedResources();

      if (bGlobalDatabase)
      {
        // These directories may have been created with 0700 permission
        // better delete them if they are empty
        QString appsDir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
        QDir().remove(appsDir);
        // was doing the same with servicetypes, but I don't think any of these gets created-by-mistake anymore.
      }
   }

   if (!parser.isSet("nosignal"))
   {
     // Notify ALL applications that have a ksycoca object, using a signal
     QDBusMessage signal = QDBusMessage::createSignal("/", "org.kde.KSycoca", "notifyDatabaseChanged" );
     signal << changedResources;

     if (QDBusConnection::sessionBus().isConnected()) {
         qDebug() << "Emitting notifyDatabaseChanged" << changedResources;
       QDBusConnection::sessionBus().send(signal);
       qApp->processEvents(); // make sure the dbus signal is sent before we quit.
     }
   }

   return 0;
}

static quint32 updateHash(const QString &file, quint32 hash)
{
    QFileInfo fi(file);
    if (fi.isReadable() && fi.isFile()) {
        // This was using buff.st_ctime (in Waldo's initial commit to kstandarddirs.cpp in 2001), but that looks wrong?
        // Surely we want to catch manual editing, while a chmod doesn't matter much?
        hash += fi.lastModified().toTime_t();
    }
    return hash;
}

quint32 KBuildSycoca::calcResourceHash(const QString& resourceSubDir, const QString &filename)
{
    quint32 hash = 0;
    if (!QDir::isRelativePath(filename))
        return updateHash(filename, hash);
    const QStringList files = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, resourceSubDir + '/' + filename);
    Q_FOREACH(const QString& file, files) {
        hash = updateHash(file, hash);
    }
    if (hash == 0 && !filename.endsWith("update_ksycoca")
        && !filename.endsWith(".directory") // bug? needs investigation from someone who understands the VFolder spec
        ) {
        qWarning() << "File not found or not readable:" << filename << "found:" << files;
        Q_ASSERT(hash != 0);
    }
    return hash;
}
