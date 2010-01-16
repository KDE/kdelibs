/*  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Waldo Bastian <bastian@kde.org>
 *  Copyright (C) 2005-2009 David Faure <faure@kde.org>
 *  Copyright (C) 2008 Hamish Rodda <rodda@kde.org>
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
#include "ksycoca_p.h"
#include "ksycocatype.h"
#include "ksycocafactory.h"
#include "ktoolinvocation.h"
#include "kglobal.h"
#include "kmemfile.h"
#include "kde_file.h"
#include "kconfiggroup.h"
#include "ksharedconfig.h"

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

#include "ksycocadevices_p.h"

// TODO: remove mmap() from kdewin32 and use QFile::mmap() when needed
#ifdef Q_WS_WIN
#undef HAVE_MMAP
#endif
/**
 * Sycoca file version number.
 * If the existing file is outdated, it will not get read
 * but instead we'll ask kded to regenerate a new one...
 */
#define KSYCOCA_VERSION 162

/**
 * Sycoca file name, used internally (by kbuildsycoca)
 */
#define KSYCOCA_FILENAME "ksycoca4"

#if HAVE_MADVISE
#include <sys/mman.h> // This #include was checked when looking for posix_madvise
#endif

#ifndef MAP_FAILED
#define MAP_FAILED ((void *) -1)
#endif

static bool s_autoRebuild = true;

// The following limitations are in place:
// Maximum length of a single string: 8192 bytes
// Maximum length of a string list: 1024 strings
// Maximum number of entries: 8192
//
// The purpose of these limitations is to limit the impact
// of database corruption.


Q_DECLARE_OPERATORS_FOR_FLAGS(KSycocaPrivate::BehaviorsIfNotFound)

KSycocaPrivate::KSycocaPrivate()
    : databaseStatus( DatabaseNotOpen ),
      readError( false ),
      timeStamp( 0 ),
      m_databasePath(),
      updateSig( 0 ),
      sycoca_size(0),
      sycoca_mmap(0),
      m_mmapFile(0),
      m_device(0)
{
#ifdef Q_OS_WIN
    /*
      on windows we use KMemFile (QSharedMemory) to avoid problems
      with mmap (can't delete a mmap'd file)
    */
    m_sycocaStrategy = StrategyMemFile;
#else
    m_sycocaStrategy = StrategyMmap;
#endif
    KConfigGroup config(KGlobal::config(), "KSycoca");
    setStrategyFromString(config.readEntry("strategy"));
}

void KSycocaPrivate::setStrategyFromString(const QString& strategy) {
    if (strategy == "mmap")
        m_sycocaStrategy = StrategyMmap;
    else if (strategy == "file")
        m_sycocaStrategy = StrategyFile;
    else if (strategy == "sharedmem")
        m_sycocaStrategy = StrategyMemFile;
    else if (!strategy.isEmpty())
        kWarning(7011) << "Unknown sycoca strategy:" << strategy;
}

bool KSycocaPrivate::tryMmap()
{
#ifdef HAVE_MMAP
    Q_ASSERT(!m_databasePath.isEmpty());
    m_mmapFile = new QFile(m_databasePath);
    const bool canRead = m_mmapFile->open(QIODevice::ReadOnly);
    Q_ASSERT(canRead);
    Q_UNUSED(canRead); // no compiler warning in release builds.
    fcntl(m_mmapFile->handle(), F_SETFD, FD_CLOEXEC);
    sycoca_size = m_mmapFile->size();
    sycoca_mmap = (const char *) mmap(0, sycoca_size,
                                      PROT_READ, MAP_SHARED,
                                      m_mmapFile->handle(), 0);
    /* POSIX mandates only MAP_FAILED, but we are paranoid so check for
       null pointer too.  */
    if (sycoca_mmap == (const char*) MAP_FAILED || sycoca_mmap == 0) {
        kDebug(7011) << "mmap failed. (length = " << sycoca_size << ")";
        sycoca_mmap = 0;
        return false;
    } else {
#ifdef HAVE_MADVISE
        (void) posix_madvise((void*)sycoca_mmap, sycoca_size, POSIX_MADV_WILLNEED);
#endif // HAVE_MADVISE
        return true;
    }
#endif // HAVE_MMAP
    return false;
}

int KSycoca::version()
{
   return KSYCOCA_VERSION;
}

class KSycocaSingleton
{
public:
    KSycocaSingleton() { }
    ~KSycocaSingleton() { }

    bool hasSycoca() const {
        return m_threadSycocas.hasLocalData();
    }
    KSycoca* sycoca() {
        if (!m_threadSycocas.hasLocalData())
            m_threadSycocas.setLocalData(new KSycoca);
        return m_threadSycocas.localData();
    }
    void setSycoca(KSycoca* s) {
        m_threadSycocas.setLocalData(s);
    }

private:
    QThreadStorage<KSycoca*> m_threadSycocas;
};

K_GLOBAL_STATIC(KSycocaSingleton, ksycocaInstance)

// Read-only constructor
KSycoca::KSycoca()
  : d(new KSycocaPrivate)
{
    QDBusConnection::sessionBus().connect(QString(), QString(),
                                          "org.kde.KSycoca", "notifyDatabaseChanged",
                                          this, SLOT(notifyDatabaseChanged(QStringList)));
}

bool KSycocaPrivate::openDatabase(bool openDummyIfNotFound)
{
    Q_ASSERT(databaseStatus == DatabaseNotOpen);

    delete m_device; m_device = 0;
    QString path = KSycoca::absoluteFilePath();

    bool canRead = KDE::access(path, R_OK) == 0;
    kDebug(7011) << "Trying to open ksycoca from " << path;
    if (!canRead) {
        path = KSycoca::absoluteFilePath(KSycoca::GlobalDatabase);
        if (!path.isEmpty()) {
            kDebug(7011) << "Trying to open global ksycoca from " << path;
            canRead = KDE::access(path, R_OK) == 0;
        }
    }

    bool result = true;
    if (canRead) {
        m_databasePath = path;
        checkVersion();
    } else { // No database file
        kDebug(7011) << "Could not open ksycoca";
        m_databasePath.clear();
        databaseStatus = NoDatabase;
        if (openDummyIfNotFound) {
            // We open a dummy database instead.
            //kDebug(7011) << "No database, opening a dummy one.";

            m_sycocaStrategy = StrategyDummyBuffer;
            QDataStream* str = stream();
            *str << qint32(KSYCOCA_VERSION);
            *str << qint32(0);
        } else {
            result = false;
        }
    }
    return result;
}

KSycocaAbstractDevice* KSycocaPrivate::device()
{
    if (m_device)
        return m_device;

    Q_ASSERT(!m_databasePath.isEmpty());

    KSycocaAbstractDevice* device = m_device;
    if (m_sycocaStrategy == StrategyDummyBuffer) {
        device = new KSycocaBufferDevice;
        device->device()->open(QIODevice::ReadOnly); // can't fail
    } else {
#ifdef HAVE_MMAP
        if (m_sycocaStrategy == StrategyMmap && tryMmap()) {
            device = new KSycocaMmapDevice(sycoca_mmap,
                                           sycoca_size);
            if (!device->device()->open(QIODevice::ReadOnly)) {
                delete device; device = 0;
            }
        }
#endif
        if (!device && m_sycocaStrategy == StrategyMemFile) {
            device = new KSycocaMemFileDevice(m_databasePath);
            if (!device->device()->open(QIODevice::ReadOnly)) {
                delete device; device = 0;
            }
        }

        if (!device) {
            device = new KSycocaFileDevice(m_databasePath);
            if (!device->device()->open(QIODevice::ReadOnly)) {
                kError(7011) << "Couldn't open" << m_databasePath << "even though it is readable? Impossible.";
                //delete device; device = 0; // this would crash in the return statement...
            }
        }
    }
    if (device) {
        m_device = device;
    }
    return m_device;
}

QDataStream*& KSycocaPrivate::stream()
{
    if (!m_device) {
        if (databaseStatus == DatabaseNotOpen) {
            checkDatabase(KSycocaPrivate::IfNotFoundRecreate | KSycocaPrivate::IfNotFoundOpenDummy);
        }

        device(); // create m_device
    }

    return m_device->stream();
}

// Read-write constructor - only for KBuildSycoca
KSycoca::KSycoca( bool /* dummy */ )
  : d(new KSycocaPrivate)
{
    // This instance was not created by the singleton, but by a direct call to new!
    ksycocaInstance->setSycoca(this);
}

KSycoca * KSycoca::self()
{
    KSycoca* s = ksycocaInstance->sycoca();
    Q_ASSERT(s);
    return s;
}

KSycoca::~KSycoca()
{
    d->closeDatabase();
    delete d;
    //if (ksycocaInstance.exists()
    //    && ksycocaInstance->self == this)
    //    ksycocaInstance->self = 0;
}

bool KSycoca::isAvailable()
{
    return self()->d->checkDatabase(KSycocaPrivate::IfNotFoundDoNothing/* don't open dummy db if not found */);
}

void KSycocaPrivate::closeDatabase()
{
    delete m_device;
    m_device = 0;

    // It is very important to delete all factories here
    // since they cache information about the database file
    // But other threads might be using them, so this class is
    // refcounted, and deleted when the last thread is done with them
    qDeleteAll(m_factories);
    m_factories.clear();
#ifdef HAVE_MMAP
    if (sycoca_mmap) {
        //QBuffer *buf = static_cast<QBuffer*>(device);
        //buf->buffer().clear();
        // Solaris has munmap(char*, size_t) and everything else should
        // be happy with a char* for munmap(void*, size_t)
        munmap(const_cast<char*>(sycoca_mmap), sycoca_size);
        sycoca_mmap = 0;
    }
    delete m_mmapFile; m_mmapFile = 0;
#endif

    databaseStatus = DatabaseNotOpen;
    timeStamp = 0;
}

void KSycoca::addFactory( KSycocaFactory *factory )
{
    d->addFactory(factory);
}

bool KSycoca::isChanged(const char *type)
{
    return self()->d->changeList.contains(type);
}

void KSycoca::notifyDatabaseChanged(const QStringList &changeList)
{
    d->changeList = changeList;
    //kDebug(7011) << QThread::currentThread() << "got a notifyDatabaseChanged signal" << changeList;
    // kbuildsycoca tells us the database file changed
    // Close the database and forget all about what we knew
    // The next call to any public method will recreate
    // everything that's needed.
    d->closeDatabase();

    // Now notify applications
    emit databaseChanged();
    emit databaseChanged(changeList);
}

QDataStream * KSycoca::findEntry(int offset, KSycocaType &type)
{
   QDataStream* str = stream();
   Q_ASSERT(str);
   //kDebug(7011) << QString("KSycoca::_findEntry(offset=%1)").arg(offset,8,16);
   str->device()->seek(offset);
   qint32 aType;
   *str >> aType;
   type = KSycocaType(aType);
   //kDebug(7011) << QString("KSycoca::found type %1").arg(aType);
   return str;
}

KSycocaFactoryList* KSycoca::factories()
{
    return d->factories();
}

// Warning, checkVersion rewinds to the beginning of stream().
bool KSycocaPrivate::checkVersion()
{
    QDataStream *m_str = device()->stream();
    Q_ASSERT(m_str);
    m_str->device()->seek(0);
    qint32 aVersion;
    *m_str >> aVersion;
    if ( aVersion < KSYCOCA_VERSION ) {
        kWarning(7011) << "Found version" << aVersion << ", expecting version" << KSYCOCA_VERSION << "or higher.";
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
    if (databaseStatus == DatabaseOK) {
        if (checkVersion()) // we know the version is ok, but we must rewind the stream anyway
            return true;
    }

    closeDatabase(); // close the dummy one

    // We can only use the installed ksycoca file if kdeinit+klauncher+kded are running,
    // since kded is what keeps the file uptodate.
    const bool kdeinitRunning = QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.klauncher");

    // Check if new database already available
    if (kdeinitRunning && openDatabase(ifNotFound & IfNotFoundOpenDummy)) {
        if (checkVersion()) {
            // Database exists, and version is ok.
            return true;
        }
    }

    if (ifNotFound & IfNotFoundRecreate) {
        // Well, if kdeinit is not running we need to launch it,
        // but otherwise we simply need to run kbuildsycoca to recreate the sycoca file.
        if (!kdeinitRunning) {
            kDebug(7011) << "We have no database.... launching kdeinit";
            KToolInvocation::klauncher(); // this calls startKdeinit, and blocks until it returns
            // and since kdeinit4 only returns after kbuildsycoca4 is done, we can proceed.
        } else {
            kDebug(7011) << QThread::currentThread() << "We have no database.... launching" << KBUILDSYCOCA_EXENAME;
            if (QProcess::execute(KStandardDirs::findExe(KBUILDSYCOCA_EXENAME)) != 0)
                qWarning("ERROR: Running KSycoca failed.");
        }

        closeDatabase(); // close the dummy one

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

    QDataStream* str = stream();
    Q_ASSERT(str);

    qint32 aId;
    qint32 aOffset;
    while(true) {
        *str >> aId;
        if (aId == 0) {
            kError(7011) << "Error, KSycocaFactory (id =" << int(id) << ") not found!";
            break;
        }
        *str >> aOffset;
        if (aId == id) {
            //kDebug(7011) << "KSycoca::findFactory(" << id << ") offset " << aOffset;
            str->device()->seek(aOffset);
            return str;
        }
    }
    return 0;
}

QString KSycoca::kfsstnd_prefixes()
{
    // do not try to launch kbuildsycoca from here; this code is also called by kbuildsycoca.
   if (!d->checkDatabase(KSycocaPrivate::IfNotFoundDoNothing))
       return QString();
   QDataStream* str = stream();
   Q_ASSERT(str);
   qint32 aId;
   qint32 aOffset;
   // skip factories offsets
   while(true)
   {
      *str >> aId;
      if ( aId )
        *str >> aOffset;
      else
        break; // just read 0
   }
   // We now point to the header
   QString prefixes;
   KSycocaEntry::read(*str, prefixes);
   *str >> d->timeStamp;
   KSycocaEntry::read(*str, d->language);
   *str >> d->updateSig;
   KSycocaEntry::read(*str, d->allResourceDirs);
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

void KSycoca::flagError()
{
    kWarning(7011) << "ERROR: KSycoca database corruption!";
    KSycocaPrivate* d = ksycocaInstance->sycoca()->d;
    if (d->readError)
        return;
    d->readError = true;
    if (s_autoRebuild) {
        // Rebuild the damned thing.
        if (QProcess::execute(KStandardDirs::findExe(KBUILDSYCOCA_EXENAME)) != 0)
            qWarning("ERROR: Running %s failed", KBUILDSYCOCA_EXENAME);
        // Old comment, maybe not true anymore:
        // Do not wait until the DBUS signal from kbuildsycoca here.
        // It deletes m_str which is a problem when flagError is called during the KSycocaFactory ctor...
    }
}

bool KSycoca::readError() // KDE5: remove
{
    return false;
}

bool KSycoca::isBuilding()
{
    return false;
}

void KSycoca::disableAutoRebuild()
{
   s_autoRebuild = false;
}

QDataStream*& KSycoca::stream()
{
    return d->stream();
}

void KSycoca::clearCaches()
{
    if (ksycocaInstance.exists() && ksycocaInstance->hasSycoca())
        ksycocaInstance->sycoca()->d->closeDatabase();
}

#include "ksycoca.moc"
