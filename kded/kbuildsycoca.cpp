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

#include <qdir.h>
#include <qeventloop.h>
#include <config.h>

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
#include <ktempfile.h>
#include <qdatastream.h>
#include <qfile.h>
#include <qtimer.h>
#include <errno.h>

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

#ifdef KBUILDSYCOCA_GUI // KBUILDSYCOCA_GUI is used on win32 to build 
                        // GUI version of kbuildsycoca, so-called "kbuildsycocaw".
# include <qlabel.h>
# include <kmessagebox.h>
  bool silent;
  bool showprogress;
#endif

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <memory> // auto_ptr

typedef Q3Dict<KSycocaEntry> KBSEntryDict;
typedef QList<KSycocaEntry::List> KSycocaEntryListList;

static quint32 newTimestamp = 0;

static KBuildServiceFactory *g_bsf = 0;
static KBuildServiceGroupFactory *g_bsgf = 0;
static KSycocaFactory *g_factory = 0;
static KCTimeInfo *g_ctimeInfo = 0;
static Q3Dict<quint32> *g_ctimeDict = 0;
static const char *g_resource = 0;
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
  QString path;

  if (bGlobalDatabase)
  {
     path = KGlobal::dirs()->saveLocation("services")+"ksycoca";
  }
  else
  {
     QByteArray ksycoca_env = getenv("KDESYCOCA");
     if (ksycoca_env.isEmpty())
        path = KGlobal::dirs()->saveLocation("cache")+"ksycoca";
     else
        path = QFile::decodeName(ksycoca_env);
  }

  return path;
}

static QString oldSycocaPath()
{
  QByteArray ksycoca_env = getenv("KDESYCOCA");
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
          app = QLatin1String(line);
          app.truncate(app.length()-1);
      }
      else if (strncmp(line+1, "mime_types=", 11) == 0)
      {
          QString mimetypes = QLatin1String(line+12);
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

KSycocaEntry *KBuildSycoca::createEntry(const QString &file, bool addToFactory)
{
   quint32 timeStamp = g_ctimeInfo->ctime(file);
   if (!timeStamp)
   {
      timeStamp = KGlobal::dirs()->calcResourceHash( g_resource, file, true);
   }
   KSycocaEntry* entry = 0;
   if (g_allEntries)
   {
      assert(g_ctimeDict);
      quint32 *timeP = (*g_ctimeDict)[file];
      quint32 oldTimestamp = timeP ? *timeP : 0;

      if (timeStamp && (timeStamp == oldTimestamp))
      {
         // Re-use old entry
         if (g_factory == g_bsgf) // Strip .directory from service-group entries
         {
            entry = g_entryDict->find(file.left(file.length()-10));
         }
         else if (g_factory == g_bsf)
         {
            entry = g_entryDict->find(file);
         }
         else
         {
            entry = g_entryDict->find(file);
         }
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
      if (addToFactory)
         g_factory->addEntry( entry, g_resource );
      else
         g_tempStorage.append(entry);
      return entry;
   }
   return 0;
}

void KBuildSycoca::slotCreateEntry(const QString &file, KService **service)
{
   KSycocaEntry *entry = createEntry(file, false);
   *service = dynamic_cast<KService *>(entry);
}

// returns false if the database is up to date
bool KBuildSycoca::build()
{
  typedef Q3PtrList<KBSEntryDict> KBSEntryDictList;
  KBSEntryDictList *entryDictList = 0;
  KBSEntryDict *serviceEntryDict = 0;

  entryDictList = new KBSEntryDictList;
  // Convert for each factory the entryList to a Dict.
  int i = 0;
  // For each factory
  for (KSycocaFactoryList::Iterator factory = m_lstFactories->begin();
       factory != m_lstFactories->end();
       ++factory)
  {
     KBSEntryDict *entryDict = new KBSEntryDict;
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
     if ((*factory) == g_bsf)
        serviceEntryDict = entryDict;
     else if ((*factory) == g_bsgf)
        g_serviceGroupEntryDict = entryDict;
     entryDictList->append(entryDict);
  }

  QStringList allResources;
  // For each factory
  for (KSycocaFactoryList::Iterator factory = m_lstFactories->begin();
       factory != m_lstFactories->end();
       ++factory)
  {
    // For each resource the factory deals with
    const KSycocaResourceList *list = (*factory)->resourceList();
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
     for (KSycocaFactoryList::Iterator it = m_lstFactories->begin();
          it != m_lstFactories->end();
	  ++it,
          g_entryDict = entryDictList->next() )
     {
        g_factory = (*it);
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
               if ((*it3).endsWith(res.extension))
                   createEntry(*it3, true);
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

  bool result = !uptodate || (g_ctimeDict && !g_ctimeDict->isEmpty());

  if (result || bMenuTest)
  {
     g_resource = "apps";
     g_factory = g_bsf;
     g_entryDict = serviceEntryDict;
     g_changed = false;

     g_vfolder = new VFolderMenu;
     if (!m_trackId.isEmpty())
        g_vfolder->setTrackId(m_trackId);

     connect(g_vfolder, SIGNAL(newService(const QString &, KService **)),
             this, SLOT(slotCreateEntry(const QString &, KService **)));
             
     VFolderMenu::SubMenu *kdeMenu = g_vfolder->parseMenu("applications.menu", true);

     KServiceGroup *entry = g_bsgf->addNew("/", kdeMenu->directoryFile, 0, false);
     entry->setLayoutInfo(kdeMenu->layoutList);
     createMenu(QString::null, QString::null, kdeMenu);

     KServiceGroup::Ptr g(entry);
     createMenuAttribute( g );

     (void) existingResourceDirs();
     *g_allResourceDirs += g_vfolder->allDirectories();

     disconnect(g_vfolder, SIGNAL(newService(const QString &, KService **)),
             this, SLOT(slotCreateEntry(const QString &, KService **)));

     if (g_changed || !g_allEntries)
     {
        uptodate = false;
        g_changeList->append(g_resource);
     }
     if (bMenuTest)
        return false;
  }

  return result;
}

void KBuildSycoca::createMenuAttribute( KServiceGroup::Ptr entry )
{
    KServiceGroup::List list = entry->entries(true, true);
    KServiceGroup::List::ConstIterator it = list.begin();
    for (; it != list.end(); ++it) {
        KSycocaEntry * e = *it;
        if (e->isType(KST_KServiceGroup)) {
            KServiceGroup::Ptr g(static_cast<KServiceGroup *>(e));
            createMenuAttribute( g );
        }
    }
}


void KBuildSycoca::createMenu(QString caption, QString name, VFolderMenu::SubMenu *menu)
{
  for(VFolderMenu::SubMenu *subMenu = menu->subMenus.first(); subMenu; subMenu = menu->subMenus.next())
  {
     QString subName = name+subMenu->name+"/";

     QString directoryFile = subMenu->directoryFile;
     if (directoryFile.isEmpty())
        directoryFile = subName+".directory";
     quint32 timeStamp = g_ctimeInfo->ctime(directoryFile);
     if (!timeStamp)
     {
        timeStamp = KGlobal::dirs()->calcResourceHash( g_resource, directoryFile, true);
     }

     KServiceGroup* entry = 0;
     if (g_allEntries)
     {
        quint32 *timeP = (*g_ctimeDict)[directoryFile];
        quint32 oldTimestamp = timeP ? *timeP : 0;

        if (timeStamp && (timeStamp == oldTimestamp))
        {
            entry = dynamic_cast<KServiceGroup *> (g_serviceGroupEntryDict->find(subName));
            if (entry && (entry->directoryEntryPath() != directoryFile))
                entry = 0; // Can't reuse this one!
        }
     }
     g_ctimeInfo->addCTime(directoryFile, timeStamp);

     entry = g_bsgf->addNew(subName, subMenu->directoryFile, entry, subMenu->isDeleted);
     entry->setLayoutInfo(subMenu->layoutList);
     if (! (bMenuTest && entry->noDisplay()) )
        createMenu(caption + entry->caption() + "/", subName, subMenu);
  }
  if (caption.isEmpty())
     caption += "/";
  if (name.isEmpty())
     name += "/";
  for(Q3DictIterator<KService> it(menu->items); it.current(); ++it)
  {
     if (bMenuTest)
     {
        if (!menu->isDeleted && !it.current()->noDisplay())
          printf("%s\t%s\t%s\n", caption.toLocal8Bit().data(), it.current()->menuId().toLocal8Bit().data(), locate("apps", it.current()->desktopEntryPath()).toLocal8Bit().data());
     }
     else
     {
        g_bsf->addEntry( it.current(), g_resource );
        g_bsgf->addNewEntryTo(name, it.current());
     }
  }
}

bool KBuildSycoca::recreate()
{
  QString path(sycocaPath());
#ifdef Q_WS_WIN
  printf("kbuildsycoca: path='%s'\n", (const char*)path);
#endif

  // KSaveFile first writes to a temp file.
  // Upon close() it moves the stuff to the right place.
  std::auto_ptr<KSaveFile> database( new KSaveFile(path) );
  if (database->status() == EACCES && QFile::exists(path))
  {
    QFile::remove( path );
    database.reset( new KSaveFile(path) ); // try again
  }
  if (database->status() != 0)
  {
    fprintf(stderr, "kbuildsycoca: ERROR creating database '%s'! %s\n", path.toLocal8Bit().data(),strerror(database->status()));
#ifdef KBUILDSYCOCA_GUI // KBUILDSYCOCA_GUI is used on win32 to build 
                        // GUI version of kbuildsycoca, so-called "kbuildsycocaw".
    if (!silent)
      KMessageBox::error(0, i18n("Error creating database '%1'.\nCheck that the permissions are correct on the directory and the disk is not full.\n").arg(path.toLocal8Bit().data()), i18n("KBuildSycoca"));
#endif
    return false;
  }

  m_str = database->dataStream();
  m_str->setVersion(QDataStream::Qt_3_1);

  kdDebug(7021) << "Recreating ksycoca file (" << path << ", version " << KSycoca::version() << ")" << endl;

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
      database->abort(); // Error
    m_str = 0L;
    if (!database->close())
    {
      fprintf(stderr, "kbuildsycoca: ERROR writing database '%s'!\n", database->name().toLocal8Bit().data());
      fprintf(stderr, "kbuildsycoca: Disk full?\n");
#ifdef KBUILDSYCOCA_GUI
      if (!silent)
        KMessageBox::error(0, i18n("Error writing database '%1'.\nCheck that the permissions are correct on the directory and the disk is not full.\n").arg(path.toLocal8Bit().data()), i18n("KBuildSycoca"));
#endif
      return false;
    }
  }
  else
  {
    m_str = 0L;
    database->abort();
    if (bMenuTest)
       return true;
    kdDebug(7021) << "Database is up to date" << endl;
  }

  if (!bGlobalDatabase)
  {
    // update the timestamp file
    QString stamppath = path + "stamp";
    QFile ksycocastamp(stamppath);
    ksycocastamp.open( QIODevice::WriteOnly );
    QDataStream str( &ksycocastamp );
    str << newTimestamp;
    str << existingResourceDirs();
    if (g_vfolder)
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
  return true;
}

void KBuildSycoca::save()
{
   // Write header (#pass 1)
   m_str->device()->at(0);

   (*m_str) << (qint32) KSycoca::version();
   KSycocaFactory * servicetypeFactory = 0L;
   KSycocaFactory * serviceFactory = 0L;
   for(KSycocaFactoryList::Iterator factory = m_lstFactories->begin();
       factory != m_lstFactories->end();
       ++factory)
   {
      qint32 aId;
      qint32 aOffset;
      aId = (*factory)->factoryId();
      if ( aId == KST_KServiceTypeFactory )
         servicetypeFactory = *factory;
      else if ( aId == KST_KServiceFactory )
         serviceFactory = *factory;
      aOffset = (*factory)->offset();
      (*m_str) << aId;
      (*m_str) << aOffset;
   }
   (*m_str) << (qint32) 0; // No more factories.
   // Write KDEDIRS
   (*m_str) << KGlobal::dirs()->kfsstnd_prefixes();
   (*m_str) << newTimestamp;
   (*m_str) << KGlobal::locale()->language();
   (*m_str) << KGlobal::dirs()->calcResourceHash("services", "update_ksycoca", true);
   (*m_str) << (*g_allResourceDirs);

   // Write factory data....
   for(KSycocaFactoryList::Iterator factory = m_lstFactories->begin();
       factory != m_lstFactories->end();
       ++factory)
   {
      (*factory)->save(*m_str);
      if (m_str->device()->status())
         return; // error
   }

   int endOfData = m_str->device()->at();

   // Write header (#pass 2)
   m_str->device()->at(0);

   (*m_str) << (qint32) KSycoca::version();
   for(KSycocaFactoryList::Iterator factory = m_lstFactories->begin();
       factory != m_lstFactories->end(); ++factory)
   {
      qint32 aId;
      qint32 aOffset;
      aId = (*factory)->factoryId();
      aOffset = (*factory)->offset();
      (*m_str) << aId;
      (*m_str) << aOffset;
   }
   (*m_str) << (qint32) 0; // No more factories.

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
   QFileInfoList list = dir.entryInfoList( QDir::NoFilter, QDir::Unsorted );
   if (list.count() == 0)
      return true;

   foreach ( QFileInfo fi, list ) {
      if( fi.fileName() == "." || fi.fileName() == ".." )
         continue;
      if( fi.lastModified() > stamp )
      {
         kdDebug( 7201 ) << "timestamp changed:" << fi.filePath() << endl;
         return false;
      }
      if( fi.isDir() && !checkDirTimestamps( fi.filePath(), stamp, false ))
            return false;
   }
   return true;
}

// check times of last modification of all files on which ksycoca depens,
// and also their directories
// if all of them all older than the timestamp in file ksycocastamp, this
// means that there's no need to rebuild ksycoca
bool KBuildSycoca::checkTimestamps( quint32 timestamp, const QStringList &dirs )
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
   { "nosignal", I18N_NOOP("Do not signal applications to update"), 0 },
   { "noincremental", I18N_NOOP("Disable incremental update, re-read everything"), 0 },
   { "checkstamps", I18N_NOOP("Check file timestamps"), 0 },
   { "nocheckfiles", I18N_NOOP("Disable checking files (dangerous)"), 0 },
   { "global", I18N_NOOP("Create global database"), 0 },
   { "menutest", I18N_NOOP("Perform menu generation test run only"), 0 },
   { "track <menu-id>", I18N_NOOP("Track menu id for debug purposes"), 0 },
#ifdef KBUILDSYCOCA_GUI
   { "silent", I18N_NOOP("Silent - work without windows and stderr"), 0 },
   { "showprogress", I18N_NOOP("Show progress information (even if 'silent' mode is on)"), 0 },
#endif
   KCmdLineLastOption
};

static const char appName[] = "kbuildsycoca";
static const char appVersion[] = "1.1";

class WaitForSignal : public QObject
{
public:
   ~WaitForSignal() { qApp->exit_loop (); }
};

extern "C" KDE_EXPORT int kdemain(int argc, char **argv)
{
   KLocale::setMainCatalog("kdelibs");
   KAboutData d(appName, I18N_NOOP("KBuildSycoca"), appVersion,
                I18N_NOOP("Rebuilds the system configuration cache."),
                KAboutData::License_GPL, "(c) 1999-2002 KDE Developers");
   d.addAuthor("David Faure", I18N_NOOP("Author"), "faure@kde.org");
   d.addAuthor("Waldo Bastian", I18N_NOOP("Author"), "bastian@kde.org");

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

   KApplication::disableAutoDcopRegistration();
#ifdef KBUILDSYCOCA_GUI
   KApplication k;
#else
   KApplication k(false, false);
#endif
   k.disableSessionManagement();

#ifdef KBUILDSYCOCA_GUI
   silent = args->isSet("silent");
   showprogress = args->isSet("showprogress");
   QLabel progress( QString("<p><br><nobr>    %1    </nobr><br>").arg( i18n("Reloading KDE configuration, please wait...") ), 0, "", Qt::WType_Dialog | Qt::WStyle_DialogBorder  | Qt::WStyle_Customize| Qt::WStyle_Title );
   QString capt = i18n("KDE Configuration Manager");
   if (!silent) {
     if (KMessageBox::No == KMessageBox::questionYesNo(0, i18n("Do you want to reload KDE configuration?"), capt, i18n("Reload"), i18n("Do Not Reload")))
       return 0;
   }
   if (!silent || showprogress) {
     progress.setCaption( capt );
     progress.show();
   }
#endif

   KCrash::setCrashHandler(KCrash::defaultCrashHandler);
   KCrash::setEmergencySaveFunction(crashHandler);
   KCrash::setApplicationName(QString(appName));

   // this program is in kdelibs so it uses kdelibs as catalog
   KLocale::setMainCatalog("kdelibs");
   // force generating of KLocale object. if not, the database will get
   // be translated
   KGlobal::locale();
   KGlobal::dirs()->addResourceType("app-reg", "share/application-registry" );

   DCOPClient *dcopClient = new DCOPClient();

   while(true)
   {
     QByteArray registeredName = dcopClient->registerAs(appName, false);
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
       obj->connect(dcopClient, SIGNAL(applicationRemoved(const QByteArray &)),
               SLOT(deleteLater()));
       qApp->enter_loop();
     }
     dcopClient->setNotifications( false );
   }
   fprintf(stderr, "%s running...\n", appName);

   bool checkfiles = bGlobalDatabase || args->isSet("checkfiles");

   bool incremental = !bGlobalDatabase && args->isSet("incremental") && checkfiles;
   if (incremental || !checkfiles)
   {
     KSycoca::self()->disableAutoRebuild(); // Prevent deadlock
     QString current_language = KGlobal::locale()->language();
     QString ksycoca_language = KSycoca::self()->language();
     quint32 current_update_sig = KGlobal::dirs()->calcResourceHash("services", "update_ksycoca", true);
     quint32 ksycoca_update_sig = KSycoca::self()->updateSignature();

     if ((current_update_sig != ksycoca_update_sig) ||
         (current_language != ksycoca_language) ||
         (KSycoca::self()->timeStamp() == 0))
     {
        incremental = false;
        checkfiles = true;
        delete KSycoca::self();
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
         qWarning("Reusing existing ksycoca");
         KSycoca *oldSycoca = KSycoca::self();
         KSycocaFactoryList *factories = new KSycocaFactoryList;
         g_allEntries = new KSycocaEntryListList;
         g_ctimeDict = new Q3Dict<quint32>(523);

         // Must be in same order as in KBuildSycoca::recreate()!
         factories->append( new KServiceTypeFactory );
         factories->append( new KServiceGroupFactory );
         factories->append( new KServiceFactory );
         factories->append( new KImageIOFactory );
         factories->append( new KProtocolInfoFactory );

         // For each factory
	 for (KSycocaFactoryList::Iterator factory = factories->begin();
	      factory != factories->end(); ++factory)
         {
             KSycocaEntry::List list;
             list = (*factory)->allEntries();
             g_allEntries->append( list );
         }
         delete factories; factories = 0;
         KCTimeInfo *ctimeInfo = new KCTimeInfo;
         ctimeInfo->fillCTimeDict(*g_ctimeDict);
         delete oldSycoca;
      }
      cSycocaPath = 0;

      KBuildSycoca *sycoca= new KBuildSycoca; // Build data base
      if (args->isSet("track"))
         sycoca->setTrackId(QString::fromLocal8Bit(args->getOption("track")));
      if (!sycoca->recreate()) {
#ifdef KBUILDSYCOCA_GUI
        if (!silent || showprogress)
          progress.close();
#endif
        return -1;
      }

      if (bGlobalDatabase)
      {
        // These directories may have been created with 0700 permission
        // better delete them if they are empty
        QString applnkDir = KGlobal::dirs()->saveLocation("apps", QString::null, false);
        ::rmdir(QFile::encodeName(applnkDir));
        QString servicetypesDir = KGlobal::dirs()->saveLocation("servicetypes", QString::null, false);
        ::rmdir(QFile::encodeName(servicetypesDir));
      }
   }

   if (args->isSet("signal"))
   {
     // Notify ALL applications that have a ksycoca object, using a broadcast
     QByteArray data;
     QDataStream stream(&data, QIODevice::WriteOnly);
     stream << *g_changeList;
     dcopClient->send( "*", "ksycoca", "notifyDatabaseChanged(QStringList)", data );
   }

#ifdef KBUILDSYCOCA_GUI
   if (!silent) {
     progress.close();
     KMessageBox::information(0, i18n("Configuration information reloaded successfully."), capt);
   }
#endif
   return 0;
}

#include "kbuildsycoca.moc"
