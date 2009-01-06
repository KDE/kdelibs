/*  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Waldo Bastian <bastian@kde.org>
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

#include "ksycoca.h"
#include "ksycocatype.h"
#include "ksycocafactory.h"
#include "ktoolinvocation.h"
#include "kglobal.h"
#include "kmemfile.h"

#include "kdebug.h"
#include "kstandarddirs.h"

#include <QtCore/QDataStream>
#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QBuffer>
#include <QProcess>
#include <QtDBus/QtDBus>

#include <config.h>

#include <stdlib.h>
#include <fcntl.h>

/**
 * Sycoca file version number.
 * If the existing file is outdated, it will not get read
 * but instead we'll ask kded to regenerate a new one...
 */
#define KSYCOCA_VERSION 140

/**
 * Sycoca file name, used internally (by kbuildsycoca)
 */
#define KSYCOCA_FILENAME "ksycoca4"

#ifdef Q_OS_WIN
/*
 on windows we use KMemFile (QSharedMemory) to avoid problems
 with mmap (can't delete a mmap'd file)
*/
#undef HAVE_MMAP
#endif

#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

#ifdef Q_OS_SOLARIS
extern "C" int madvise(caddr_t, size_t, int);
#endif

#ifndef MAP_FAILED
#define MAP_FAILED ((void *) -1)
#endif

// The following limitations are in place:
// Maximum length of a single string: 8192 bytes
// Maximum length of a string list: 1024 strings
// Maximum number of entries: 8192
//
// The purpose of these limitations is to limit the impact
// of database corruption.

class KSycocaPrivate
{
public:
    KSycocaPrivate()
        : databaseStatus( DatabaseNotOpen ),
          readError( false ),
          autoRebuild( true ),
          sycoca_size( 0 ),
          sycoca_mmap( 0 ),
          timeStamp( 0 ),
          m_database( 0 ),
          m_dummyBuffer(0),
          updateSig( 0 ),
          lstFactories( 0 )
    {
    }

    static void delete_ksycoca_self() {
        delete _self;
        _self = 0;
    }

    bool checkVersion();
    bool openDatabase(bool openDummyIfNotFound=true);
    enum BehaviorIfNotFound {
        IfNotFoundDoNothing = 0,
        IfNotFoundOpenDummy = 1,
        IfNotFoundRecreate = 2
    };
    Q_DECLARE_FLAGS(BehaviorsIfNotFound, BehaviorIfNotFound)
    bool checkDatabase(BehaviorsIfNotFound ifNotFound);
    void closeDatabase();

    enum {
        DatabaseNotOpen, // m_str is 0, openDatabase must be called
        NoDatabase, // not found, so we opened a dummy one instead
        BadVersion, // it's opened, but it's not useable
        DatabaseOK } databaseStatus;
    bool readError;
    bool autoRebuild;
    size_t sycoca_size;
    const char *sycoca_mmap;
    quint32 timeStamp;
#ifdef Q_OS_WIN
    KMemFile *m_database;
#else
    QFile *m_database;
#endif
    QBuffer* m_dummyBuffer;
    QStringList changeList;
    QString language;
    quint32 updateSig;
    QStringList allResourceDirs;
    KSycocaFactoryList *lstFactories;
    static KSycoca *_self;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(KSycocaPrivate::BehaviorsIfNotFound)

KSycoca * KSycocaPrivate::_self = 0L;

int KSycoca::version()
{
   return KSYCOCA_VERSION;
}

// Read-only constructor
KSycoca::KSycoca()
  : m_str(0),
    d(new KSycocaPrivate)
{
   QDBusConnection::sessionBus().connect(QString(), QString(), "org.kde.KSycoca", "notifyDatabaseChanged",
                               this, SLOT(notifyDatabaseChanged(QStringList)));
   KSycocaPrivate::_self = this;
   // We register with D-Bus _before_ we try to open the database.
   // This way we can be relatively sure that the KDE framework is
   // up and running (kdeinit, klauncher, kded) and
   // that the database is up to date.

   //   -> huh? -thiago
   //   This is because dcopserver was autostarted (via kdeinit) when trying to register to dcop. - David
   //   But the "launching kdeinit" case below takes care of it.
   d->openDatabase();
}

bool KSycocaPrivate::openDatabase( bool openDummyIfNotFound )
{
   bool result = true;

   sycoca_mmap = 0;
   QDataStream* &m_str = KSycocaPrivate::_self->m_str;
   m_str = 0;
   delete m_dummyBuffer;
   m_dummyBuffer = 0;
   QString path = KSycoca::absoluteFilePath();

   kDebug(7011) << "Trying to open ksycoca from " << path;
#ifdef Q_OS_WIN
   m_database = new KMemFile(path);
#else
   m_database = new QFile(path);
#endif
   bool bOpen = m_database->open( QIODevice::ReadOnly );
   if (!bOpen)
   {
     path = KSycoca::absoluteFilePath(KSycoca::GlobalDatabase);
     if (!path.isEmpty())
     {
       kDebug(7011) << "Trying to open global ksycoca from " << path;
       delete m_database;
#ifdef Q_OS_WIN
       m_database = new KMemFile(path);
#else
       m_database = new QFile(path);
#endif
       bOpen = m_database->open( QIODevice::ReadOnly );
     }
   }

   if (bOpen)
   {
#ifdef Q_OS_WIN
     m_str = new QDataStream(m_database);
     m_str->setVersion(QDataStream::Qt_3_1);
     sycoca_mmap = 0;
#else // Q_OS_WIN
     fcntl(m_database->handle(), F_SETFD, FD_CLOEXEC);
     sycoca_size = m_database->size();
#ifdef HAVE_MMAP
     sycoca_mmap = (const char *) mmap(0, sycoca_size,
                                       PROT_READ, MAP_SHARED,
                                       m_database->handle(), 0);
     /* POSIX mandates only MAP_FAILED, but we are paranoid so check for
        null pointer too.  */
     if (sycoca_mmap == (const char*) MAP_FAILED || sycoca_mmap == 0)
     {
        kDebug(7011) << "mmap failed. (length = " << sycoca_size << ")";
#endif // HAVE_MMAP
        m_str = new QDataStream(m_database);
        m_str->setVersion(QDataStream::Qt_3_1);
        sycoca_mmap = 0;
#ifdef HAVE_MMAP
     }
     else
     {
#ifdef HAVE_MADVISE
        (void) madvise((char*)sycoca_mmap, sycoca_size, MADV_WILLNEED);
#endif // HAVE_MADVISE
        m_dummyBuffer = new QBuffer;
        m_dummyBuffer->setData(QByteArray::fromRawData(sycoca_mmap, sycoca_size));
        m_dummyBuffer->open(QIODevice::ReadOnly);
        m_str = new QDataStream(m_dummyBuffer);
        m_str->setVersion(QDataStream::Qt_3_1);
     }
#endif // HAVE_MMAP
#endif // !Q_OS_WIN
     checkVersion();
   }
   else
   {
     kDebug(7011) << "Could not open ksycoca";

     // No database file
     delete m_database;
     m_database = 0;

     databaseStatus = NoDatabase;
     if (openDummyIfNotFound)
     {
        // We open a dummy database instead.
        //kDebug(7011) << "No database, opening a dummy one.";
        m_dummyBuffer = new QBuffer;
        m_dummyBuffer->open(QIODevice::ReadWrite);
        m_str = new QDataStream(m_dummyBuffer);
        m_str->setVersion(QDataStream::Qt_3_1);
        *m_str << qint32(KSYCOCA_VERSION);
        *m_str << qint32(0);
     }
     else
     {
        result = false;
     }
   }
   lstFactories = new KSycocaFactoryList;
   return result;
}

// Read-write constructor - only for KBuildSycoca
KSycoca::KSycoca( bool /* dummy */ )
  : m_str(0),
    d(new KSycocaPrivate)
{
   QDBusConnection::sessionBus().registerObject("/ksycoca_building", this, QDBusConnection::ExportScriptableSlots);
   d->lstFactories = new KSycocaFactoryList;
   KSycocaPrivate::_self = this;
}

KSycoca * KSycoca::self()
{
    if (!KSycocaPrivate::_self) {
        qAddPostRoutine(KSycocaPrivate::delete_ksycoca_self);
        KSycocaPrivate::_self = new KSycoca;
    }
    return KSycocaPrivate::_self;
}

KSycoca::~KSycoca()
{
   d->closeDatabase();
   delete d;
   KSycocaPrivate::_self = 0L;
}

bool KSycoca::isAvailable()
{
    return self()->d->checkDatabase(KSycocaPrivate::IfNotFoundDoNothing/* don't open dummy db if not found */);
}

void KSycocaPrivate::closeDatabase()
{
   QDataStream* &m_str = KSycocaPrivate::_self->m_str;
   QIODevice *device = 0;
   if (m_str)
      device = m_str->device();
#ifdef HAVE_MMAP
   if (device && sycoca_mmap)
   {
      QBuffer *buf = static_cast<QBuffer*>(device);
      buf->buffer().clear();
      // Solaris has munmap(char*, size_t) and everything else should
      // be happy with a char* for munmap(void*, size_t)
      munmap(const_cast<char*>(sycoca_mmap), sycoca_size);
      sycoca_mmap = 0;
   }
#endif

   delete m_dummyBuffer;
   m_dummyBuffer = 0;
   if (m_database != device)
      delete m_database;
   device = 0;
   m_database = 0;
   delete m_str;
   m_str = 0;
   // It is very important to delete all factories here
   // since they cache information about the database file
   if ( lstFactories )
       qDeleteAll( *lstFactories );
   delete lstFactories;
   lstFactories = 0;
   databaseStatus = DatabaseNotOpen;
}

void KSycoca::addFactory( KSycocaFactory *factory )
{
   Q_ASSERT(d->lstFactories != 0);
   d->lstFactories->append(factory);
}

bool KSycoca::isChanged(const char *type)
{
    return self()->d->changeList.contains(type);
}

void KSycoca::notifyDatabaseChanged(const QStringList &changeList)
{
    d->changeList = changeList;
    //kDebug() << "got a notifyDatabaseChanged signal" << changeList;
    // kded tells us the database file changed
    // Close the database and forget all about what we knew
    // The next call to any public method will recreate
    // everything that's needed.
    d->closeDatabase();

    // Now notify applications
    emit databaseChanged();
}

QDataStream * KSycoca::findEntry(int offset, KSycocaType &type)
{
   if ( !m_str )
       d->checkDatabase(KSycocaPrivate::IfNotFoundRecreate | KSycocaPrivate::IfNotFoundOpenDummy);
   Q_ASSERT(m_str);
   //kDebug(7011) << QString("KSycoca::_findEntry(offset=%1)").arg(offset,8,16);
   m_str->device()->seek(offset);
   qint32 aType;
   *m_str >> aType;
   type = KSycocaType(aType);
   //kDebug(7011) << QString("KSycoca::found type %1").arg(aType);
   return m_str;
}

KSycocaFactoryList* KSycoca::factories()
{
    return d->lstFactories;
}

// Warning, checkVersion rewinds to the beginning of m_str.
bool KSycocaPrivate::checkVersion()
{
    QDataStream *m_str = KSycocaPrivate::_self->m_str;
    Q_ASSERT(m_str);
    m_str->device()->seek(0);
    qint32 aVersion;
    *m_str >> aVersion;
    if ( aVersion < KSYCOCA_VERSION ) {
        kWarning(7011) << "Found version " << aVersion << ", expecting version " << KSYCOCA_VERSION << " or higher.";
        databaseStatus = BadVersion;
        return false;
    } else {
        databaseStatus = DatabaseOK;
        return true;
    }
}

// If it returns true, we have a valid database and the stream has rewinded to the beginning
// and past the version number.
bool KSycocaPrivate::checkDatabase(BehaviorsIfNotFound ifNotFound)
{
    QDataStream* &m_str = KSycocaPrivate::_self->m_str;
    if (databaseStatus == DatabaseOK) {
        Q_ASSERT(m_str);
        if (checkVersion()) // we know the version is ok, but we must rewind the stream anyway
            return true;
    }

    closeDatabase(); // close the dummy one
    // Check if new database already available
    if( openDatabase(ifNotFound & IfNotFoundOpenDummy) ) {
        Q_ASSERT(m_str); // if a database was found then m_str shouldn't be 0
        if (checkVersion()) {
            // Database exists, and version is ok.
            return true;
        }
    }

    static bool triedLaunchingKdeinit = false;
    if ((ifNotFound & IfNotFoundRecreate) && !triedLaunchingKdeinit) { // try only once
        triedLaunchingKdeinit = true;
        // Well, if kdeinit is not running we need to launch it,
        // but otherwise we simply need to run kbuildsycoca to recreate the sycoca file.
        if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.klauncher")) {
            kDebug(7011) << "We have no database.... launching kdeinit";
            KToolInvocation::klauncher(); // this calls startKdeinit
        } else {
            kDebug(7011) << "We have no database.... launching " << KBUILDSYCOCA_EXENAME;
            if (QProcess::execute(KStandardDirs::findExe(KBUILDSYCOCA_EXENAME)) != 0)
                qWarning("ERROR: Running KSycoca failed.");
        }

        // Wait until the DBUS signal from kbuildsycoca
        QEventLoop eventLoop;
        QObject::connect(KSycoca::self(), SIGNAL(databaseChanged()), &eventLoop, SLOT(quit()));
        eventLoop.exec( QEventLoop::ExcludeUserInputEvents );

        // Ok, the new database should be here now, open it.
        if (!openDatabase(ifNotFound & IfNotFoundOpenDummy)) {
            kDebug(7011) << "Still no database...";
            return false; // Still no database - uh oh
        }
        if (!checkVersion()) {
            kDebug(7011) << "Still outdated...";
            return false; // Still outdated - uh oh
        }
        return true;
    }

    return false;
}

QDataStream * KSycoca::findFactory(KSycocaFactoryId id)
{
    // Ensure we have a valid database (right version, and rewinded to beginning)
    if (!d->checkDatabase(KSycocaPrivate::IfNotFoundRecreate)) {
        return 0;
    }

    qint32 aId;
    qint32 aOffset;
    while(true) {
        *m_str >> aId;
        if (aId == 0) {
            kError(7011) << "Error, KSycocaFactory (id = " << int(id) << ") not found!" << endl;
            break;
        }
        *m_str >> aOffset;
        if (aId == id) {
            //kDebug(7011) << "KSycoca::findFactory(" << id << ") offset " << aOffset;
            m_str->device()->seek(aOffset);
            return m_str;
        }
    }
    return 0;
}

QString KSycoca::kfsstnd_prefixes()
{
    // do not try to launch kbuildsycoca from here; this code is also called by kbuildsycoca.
   if (!d->checkDatabase(KSycocaPrivate::IfNotFoundDoNothing)) return "";
   qint32 aId;
   qint32 aOffset;
   // skip factories offsets
   while(true)
   {
      *m_str >> aId;
      if ( aId )
        *m_str >> aOffset;
      else
        break; // just read 0
   }
   // We now point to the header
   QString prefixes;
   KSycocaEntry::read(*m_str, prefixes);
   *m_str >> d->timeStamp;
   KSycocaEntry::read(*m_str, d->language);
   *m_str >> d->updateSig;
   KSycocaEntry::read(*m_str, d->allResourceDirs);
   return prefixes;
}

quint32 KSycoca::timeStamp()
{
   if (!d->timeStamp)
      (void) kfsstnd_prefixes();
   return d->timeStamp;
}

quint32 KSycoca::updateSignature()
{
   if (!d->timeStamp)
      (void) kfsstnd_prefixes();
   return d->updateSig;
}

QString KSycoca::absoluteFilePath(DatabaseType type)
{
   if (type == GlobalDatabase)
      return KStandardDirs::locate("services", KSYCOCA_FILENAME);

   const QByteArray ksycoca_env = qgetenv("KDESYCOCA");
   if (ksycoca_env.isEmpty())
      return KGlobal::dirs()->saveLocation("cache") + KSYCOCA_FILENAME;
   else
      return QFile::decodeName(ksycoca_env);
}

QString KSycoca::language()
{
   if (d->language.isEmpty())
      (void) kfsstnd_prefixes();
   return d->language;
}

QStringList KSycoca::allResourceDirs()
{
   if (!d->timeStamp)
      (void) kfsstnd_prefixes();
   return d->allResourceDirs;
}

#if 0
QString KSycoca::determineRelativePath( const QString & _fullpath, const char *_resource )
{
  QString sRelativeFilePath;
  QStringList dirs = KGlobal::dirs()->resourceDirs( _resource );
  QStringList::ConstIterator dirsit = dirs.begin();
  for ( ; dirsit != dirs.end() && sRelativeFilePath.isEmpty(); ++dirsit ) {
    // might need canonicalPath() ...
    if ( _fullpath.indexOf( *dirsit ) == 0 ) // path is dirs + relativePath
      sRelativeFilePath = _fullpath.mid( (*dirsit).length() ); // skip appsdirs
  }
  if ( sRelativeFilePath.isEmpty() )
    kFatal(7011) << QString("Couldn't find %1 in any %2 dir !!!").arg( _fullpath ).arg( _resource);
  //else
    // debug code
    //kDebug(7011) << sRelativeFilePath;
  return sRelativeFilePath;
}
#endif

void KSycoca::flagError()
{
    kWarning(7011) << "ERROR: KSycoca database corruption!";
   if (KSycocaPrivate::_self)
   {
      if (KSycocaPrivate::_self->d->readError)
         return;
      KSycocaPrivate::_self->d->readError = true;
      if (KSycocaPrivate::_self->d->autoRebuild) {
          // Rebuild the damned thing.
          if (QProcess::execute(KStandardDirs::findExe(KBUILDSYCOCA_EXENAME)) != 0)
              qWarning("ERROR: Running %s failed", KBUILDSYCOCA_EXENAME);
          // Do not wait until the DBUS signal from kbuildsycoca here.
          // It deletes m_str which is a problem when flagError is called during the KSycocaFactory ctor...
      }
   }
}

bool KSycoca::isBuilding()
{
    return false;
}

void KSycoca::disableAutoRebuild()
{
   d->autoRebuild = false;
}

bool KSycoca::readError()
{
   bool b = false;
   if (KSycocaPrivate::_self)
   {
      b = KSycocaPrivate::_self->d->readError;
      KSycocaPrivate::_self->d->readError = false;
   }
   return b;
}

#include "ksycoca.moc"
