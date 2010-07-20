/*
   This file is part of the KDE libraries
   Copyright (c) 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1997-1999 Matthias Kalle Dalheimer <kalle@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kconfig.h"
#include "kconfig_p.h"

#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>

#include "kconfigbackend.h"
#include "kconfiggroup.h"
#include <kstringhandler.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kcomponentdata.h>
#include <ktoolinvocation.h>
#include <kaboutdata.h>
#include <kdebug.h>

#include <qbytearray.h>
#include <qfile.h>
#include <qdir.h>
#include <qdatetime.h>
#include <qrect.h>
#include <qsize.h>
#include <qcolor.h>
#include <QtCore/QProcess>
#include <QtCore/QPointer>
#include <QtCore/QSet>
#include <QtCore/QStack>

bool KConfigPrivate::mappingsRegistered=false;

KConfigPrivate::KConfigPrivate(const KComponentData &componentData_, KConfig::OpenFlags flags,
                               const char* resource)
    : openFlags(flags), resourceType(resource), mBackend(0),
      bDynamicBackend(true),  bDirty(false), bReadDefaults(false),
      bFileImmutable(false), bForceGlobal(false), bSuppressGlobal(false),
      componentData(componentData_), configState(KConfigBase::NoAccess)
{
    sGlobalFileName = componentData.dirs()->saveLocation("config") + QLatin1String("kdeglobals");

    static int use_etc_kderc = -1;
    if (use_etc_kderc < 0)
        use_etc_kderc = getenv("KDE_SKIP_KDERC") != 0 ? 0 : 1; // for unit tests
    if (use_etc_kderc) {

        etc_kderc =
#ifdef Q_WS_WIN
            QFile::decodeName( qgetenv("WINDIR") + "/kde4rc" );
#else
            QLatin1String("/etc/kde4rc");
#endif
        if (!KStandardDirs::checkAccess(etc_kderc, R_OK)) {
            etc_kderc.clear();
        }
    }

//    if (!mappingsRegistered) {
//        KEntryMap tmp;
//        if (!etc_kderc.isEmpty()) {
//            KSharedPtr<KConfigBackend> backend = KConfigBackend::create(componentData, etc_kderc, QLatin1String("INI"));
//            backend->parseConfig( "en_US", tmp, KConfigBackend::ParseDefaults);
//        }
//        const QString kde4rc(QDir::home().filePath(".kde4rc"));
//        if (KStandardDirs::checkAccess(kde4rc, R_OK)) {
//            KSharedPtr<KConfigBackend> backend = KConfigBackend::create(componentData, kde4rc, QLatin1String("INI"));
//            backend->parseConfig( "en_US", tmp, KConfigBackend::ParseOptions());
//        }
//        KConfigBackend::registerMappings(tmp);
//        mappingsRegistered = true;
//    }

    setLocale(KGlobal::hasLocale() ? KGlobal::locale()->language() : KLocale::defaultLanguage());
}


bool KConfigPrivate::lockLocal()
{
    if (mBackend) {
        return mBackend->lock(componentData);
    }
    // anonymous object - pretend we locked it
    return true;
}

void KConfigPrivate::copyGroup(const QByteArray& source, const QByteArray& destination,
                                KConfigGroup *otherGroup, KConfigBase::WriteConfigFlags flags) const
{
    KEntryMap& otherMap = otherGroup->config()->d_ptr->entryMap;
    const int len = source.length();
    const bool sameName = (destination == source);

    // we keep this bool outside the foreach loop so that if
    // the group is empty, we don't end up marking the other config
    // as dirty erroneously
    bool dirtied = false;

    for (KEntryMap::ConstIterator entryMapIt( entryMap.constBegin() ); entryMapIt != entryMap.constEnd(); ++entryMapIt) {
        const QByteArray& group = entryMapIt.key().mGroup;

        if (!group.startsWith(source)) // nothing to do
            continue;

        // don't copy groups that start with the same prefix, but are not sub-groups
        if (group.length() > len && group[len] != '\x1d')
            continue;

        KEntryKey newKey = entryMapIt.key();

        if (flags & KConfigBase::Localized) {
            newKey.bLocal = true;
        }

        if (!sameName)
            newKey.mGroup.replace(0, len, destination);

        KEntry entry = entryMap[ entryMapIt.key() ];
        dirtied = entry.bDirty = flags & KConfigBase::Persistent;

        if (flags & KConfigBase::Global) {
            entry.bGlobal = true;
        }

        otherMap[newKey] = entry;
    }

    if (dirtied) {
        otherGroup->config()->d_ptr->bDirty = true;
    }
}

KConfig::KConfig( const QString& file, OpenFlags mode,
                  const char* resourceType)
  : d_ptr(new KConfigPrivate(KGlobal::mainComponent(), mode, resourceType))
{
    d_ptr->changeFileName(file, resourceType); // set the local file name

    // read initial information off disk
    reparseConfiguration();
}

KConfig::KConfig( const KComponentData& componentData, const QString& file, OpenFlags mode,
                  const char* resourceType)
    : d_ptr(new KConfigPrivate(componentData, mode, resourceType))
{
    d_ptr->changeFileName(file, resourceType); // set the local file name

    // read initial information off disk
    reparseConfiguration();
}

KConfig::KConfig(const QString& file, const QString& backend, const char* resourceType)
    : d_ptr(new KConfigPrivate(KGlobal::mainComponent(), SimpleConfig, resourceType))
{
    d_ptr->mBackend = KConfigBackend::create(d_ptr->componentData, file, backend);
    d_ptr->bDynamicBackend = false;
    d_ptr->changeFileName(file, ""); // set the local file name

    // read initial information off disk
    reparseConfiguration();
}

KConfig::KConfig(KConfigPrivate &d)
    : d_ptr(&d)
{
}

KConfig::~KConfig()
{
    Q_D(KConfig);
    if (d->bDirty && d->mBackend.isUnique())
        sync();
    delete d;
}

const KComponentData& KConfig::componentData() const
{
    Q_D(const KConfig);
    return d->componentData;
}

QStringList KConfig::groupList() const
{
    Q_D(const KConfig);
    QSet<QString> groups;

    for (KEntryMap::ConstIterator entryMapIt( d->entryMap.constBegin() ); entryMapIt != d->entryMap.constEnd(); ++entryMapIt) {
        const QByteArray group = entryMapIt.key().mGroup;
        if (entryMapIt.key().mKey.isNull() && !group.isEmpty() && group != "<default>" && group != "$Version") {
            QString groupname = QString::fromUtf8(group);
            groups << groupname.left(groupname.indexOf('\x1d'));
        }
    }

    return groups.toList();
}

QStringList KConfigPrivate::groupList(const QByteArray& group) const
{
    QByteArray theGroup = group + '\x1d';
    QSet<QString> groups;

    for (KEntryMap::ConstIterator entryMapIt( entryMap.constBegin() ); entryMapIt != entryMap.constEnd(); ++entryMapIt)
        if (entryMapIt.key().mKey.isNull() && entryMapIt.key().mGroup.startsWith(theGroup)) {
            QString groupname = QString::fromUtf8(entryMapIt.key().mGroup.mid(theGroup.length()));
            groups << groupname.left(groupname.indexOf('\x1d'));
        }

    return groups.toList();
}

QStringList KConfig::keyList(const QString& aGroup) const
{
    Q_D(const KConfig);
    QStringList keys;
    const QByteArray theGroup(aGroup.isEmpty() ? "<default>" : aGroup.toUtf8());

    const KEntryMapConstIterator theEnd = d->entryMap.constEnd();
    KEntryMapConstIterator it = d->entryMap.findEntry(theGroup);
    if (it != theEnd) {
        ++it; // advance past the special group entry marker

        QSet<QString> tmp;
        for (; it != theEnd && it.key().mGroup == theGroup; ++it) {
            const KEntryKey& key = it.key();
            if (key.mGroup == theGroup && !key.mKey.isNull() && !it->bDeleted)
                tmp << QString::fromUtf8(key.mKey);
        }
        keys = tmp.toList();
    }

    return keys;
}

QMap<QString,QString> KConfig::entryMap(const QString& aGroup) const
{
    Q_D(const KConfig);
    QMap<QString, QString> theMap;
    const QByteArray theGroup(aGroup.isEmpty() ? "<default>" : aGroup.toUtf8());

    const KEntryMapConstIterator theEnd = d->entryMap.constEnd();
    KEntryMapConstIterator it = d->entryMap.findEntry(theGroup, 0, 0);
    if (it != theEnd) {
        ++it; // advance past the special group entry marker

        for (; it != theEnd && it.key().mGroup == theGroup; ++it) {
            // leave the default values and deleted entries out
            if (!it->bDeleted && !it.key().bDefault) {
                const QString key = QString::fromUtf8(it.key().mKey.constData());
                // the localized entry should come first, so don't overwrite it
                // with the non-localized entry
                if (!theMap.contains(key))
                    theMap.insert(key,QString::fromUtf8(it->mValue.constData()));
            }
        }
    }

    return theMap;
}

// TODO KDE5: return a bool value
void KConfig::sync()
{
    Q_D(KConfig);

    if (isImmutable() || name().isEmpty()) {
        // can't write to an immutable or anonymous file.
        return;
    }

    if (d->bDirty && d->mBackend) {
        const QByteArray utf8Locale(locale().toUtf8());

        // Create the containing dir, maybe it wasn't there
        d->mBackend->createEnclosing();

        // lock the local file
        if (d->configState == ReadWrite && !d->lockLocal()) {
            qWarning() << "couldn't lock local file";
            return;
        }

        // Rewrite global/local config only if there is a dirty entry in it.
        bool writeGlobals = false;
        bool writeLocals = false;
        foreach (const KEntry& e, d->entryMap) {
            if (e.bDirty) {
                if (e.bGlobal) {
                    writeGlobals = true;
                } else {
                    writeLocals = true;
                }

                if (writeGlobals && writeLocals) {
                    break;
                }
            }
        }

        d->bDirty = false; // will revert to true if a config write fails

        if (d->wantGlobals() && writeGlobals) {
            KSharedPtr<KConfigBackend> tmp = KConfigBackend::create(componentData(), d->sGlobalFileName);
            if (d->configState == ReadWrite && !tmp->lock(componentData())) {
                qWarning() << "couldn't lock global file";
                return;
            }
            if (!tmp->writeConfig(utf8Locale, d->entryMap, KConfigBackend::WriteGlobal, d->componentData)) {
                d->bDirty = true;
                // TODO KDE5: return false? (to tell the app that writing wasn't possible, e.g.
                // config file is immutable or disk full)
            }
            if (tmp->isLocked()) {
                tmp->unlock();
            }
        }

        if (writeLocals) {
            if (!d->mBackend->writeConfig(utf8Locale, d->entryMap, KConfigBackend::WriteOptions(), d->componentData)) {
                d->bDirty = true;
                // TODO KDE5: return false? (to tell the app that writing wasn't possible, e.g.
                // config file is immutable or disk full)
            }
        }
        if (d->mBackend->isLocked()) {
            d->mBackend->unlock();
        }
    }
}

void KConfig::markAsClean()
{
    Q_D(KConfig);
    d->bDirty = false;

    // clear any dirty flags that entries might have set
    const KEntryMapIterator theEnd = d->entryMap.end();
    for (KEntryMapIterator it = d->entryMap.begin(); it != theEnd; ++it)
        it->bDirty = false;
}

void KConfig::checkUpdate(const QString &id, const QString &updateFile)
{
    const KConfigGroup cg(this, "$Version");
    const QString cfg_id = updateFile+':'+id;
    const QStringList ids = cg.readEntry("update_info", QStringList());
    if (!ids.contains(cfg_id)) {
        KToolInvocation::kdeinitExecWait("kconf_update", QStringList() << "--check" << updateFile);
        reparseConfiguration();
    }
}

KConfig* KConfig::copyTo(const QString &file, KConfig *config) const
{
    Q_D(const KConfig);
    if (!config)
        config = new KConfig(componentData(), QString(), SimpleConfig);
    config->d_func()->changeFileName(file, d->resourceType);
    config->d_func()->entryMap = d->entryMap;
    config->d_func()->bFileImmutable = false;

    const KEntryMapIterator theEnd = config->d_func()->entryMap.end();
    for (KEntryMapIterator it = config->d_func()->entryMap.begin(); it != theEnd; ++it)
        it->bDirty = true;
    config->d_ptr->bDirty = true;

    return config;
}

QString KConfig::name() const
{
    Q_D(const KConfig);
    return d->fileName;
}

void KConfigPrivate::changeFileName(const QString& name, const char* type)
{
    fileName = name;

    QString file;
    if (name.isEmpty()) {
        if (wantDefaults()) { // accessing default app-specific config "appnamerc"
            const QString appName = componentData.aboutData()->appName();
            if (!appName.isEmpty()) {
                fileName = appName + QLatin1String("rc");
                if (type && *type)
                    resourceType = type; // only change it if it's not empty
                file = KStandardDirs::locateLocal(resourceType, fileName, false, componentData);
            }
        } else if (wantGlobals()) { // accessing "kdeglobals" - XXX used anywhere?
            resourceType = "config";
            fileName = QLatin1String("kdeglobals");
            file = sGlobalFileName;
        } // else anonymous config.
        // KDE5: remove these magic overloads
    } else if (QDir::isAbsolutePath(fileName))
        file = fileName;
    else {
        if (type && *type)
            resourceType = type; // only change it if it's not empty
        file = KStandardDirs::locateLocal(resourceType, fileName, false, componentData);
    }

    if (file.isEmpty()) {
        openFlags = KConfig::SimpleConfig;
        return;
    }

    bSuppressGlobal = (file == sGlobalFileName);

    if (bDynamicBackend || !mBackend) // allow dynamic changing of backend
        mBackend = KConfigBackend::create(componentData, file);
    else
        mBackend->setFilePath(file);

    configState = mBackend->accessMode();
}

void KConfig::reparseConfiguration()
{
    Q_D(KConfig);
    if (d->fileName.isEmpty()) {
        return;
    }

    // Don't lose pending changes
    if (!d->isReadOnly() && d->bDirty)
        sync();

    d->entryMap.clear();

    d->bFileImmutable = false;

    // Parse all desired files from the least to the most specific.
    if (d->wantGlobals())
        d->parseGlobalFiles();

    d->parseConfigFiles();
}


QStringList KConfigPrivate::getGlobalFiles() const
{
    const KStandardDirs *const dirs = componentData.dirs();
    QStringList globalFiles;
    foreach (const QString& dir1, dirs->findAllResources("config", QLatin1String("kdeglobals")))
        globalFiles.push_front(dir1);
    foreach (const QString& dir2, dirs->findAllResources("config", QLatin1String("system.kdeglobals")))
        globalFiles.push_front(dir2);
    if (!etc_kderc.isEmpty())
        globalFiles.push_front(etc_kderc);
    return globalFiles;
}

void KConfigPrivate::parseGlobalFiles()
{
    const QStringList globalFiles = getGlobalFiles();
//    qDebug() << "parsing global files" << globalFiles;

    // TODO: can we cache the values in etc_kderc / other global files
    //       on a per-application basis?
    const QByteArray utf8Locale = locale.toUtf8();
    foreach(const QString& file, globalFiles) {
        KConfigBackend::ParseOptions parseOpts = KConfigBackend::ParseGlobal|KConfigBackend::ParseExpansions;
        if (file != sGlobalFileName)
            parseOpts |= KConfigBackend::ParseDefaults;

        KSharedPtr<KConfigBackend> backend = KConfigBackend::create(componentData, file);
        if ( backend->parseConfig( utf8Locale, entryMap, parseOpts) == KConfigBackend::ParseImmutable)
            break;
    }
}

void KConfigPrivate::parseConfigFiles()
{
    // can only read the file if there is a backend and a file name
    if (mBackend && !fileName.isEmpty()) {

        bFileImmutable = false;

        QList<QString> files;
        if (wantDefaults()) {
            if (bSuppressGlobal) {
                files = getGlobalFiles();
            } else {
                foreach (const QString& f, componentData.dirs()->findAllResources(
                                                    resourceType, fileName))
                    files.prepend(f);
            }
        } else {
            files << mBackend->filePath();
        }
        if (!isSimple())
            files = extraFiles.toList() + files;

//        qDebug() << "parsing local files" << files;

        const QByteArray utf8Locale = locale.toUtf8();
        foreach(const QString& file, files) {
            if (file == mBackend->filePath()) {
                switch (mBackend->parseConfig(utf8Locale, entryMap, KConfigBackend::ParseExpansions)) {
                case KConfigBackend::ParseOk:
                    break;
                case KConfigBackend::ParseImmutable:
                    bFileImmutable = true;
                    break;
                case KConfigBackend::ParseOpenError:
                    configState = KConfigBase::NoAccess;
                    break;
                }
            } else {
                KSharedPtr<KConfigBackend> backend = KConfigBackend::create(componentData, file);
                bFileImmutable = (backend->parseConfig(utf8Locale, entryMap,
                                        KConfigBackend::ParseDefaults|KConfigBackend::ParseExpansions)
                                  == KConfigBackend::ParseImmutable);
            }

            if (bFileImmutable)
                break;
        }
        if (componentData.dirs()->isRestrictedResource(resourceType, fileName))
            bFileImmutable = true;
    }
}

KConfig::AccessMode KConfig::accessMode() const
{
    Q_D(const KConfig);
    return d->configState;
}

void KConfig::addConfigSources(const QStringList& files)
{
    Q_D(KConfig);
    foreach(const QString& file, files) {
        d->extraFiles.push(file);
    }

    if (!files.isEmpty()) {
        reparseConfiguration();
    }
}

QString KConfig::locale() const
{
    Q_D(const KConfig);
    return d->locale;
}

bool KConfigPrivate::setLocale(const QString& aLocale)
{
    if (aLocale != locale) {
        locale = aLocale;
        return true;
    }
    return false;
}

bool KConfig::setLocale(const QString& locale)
{
    Q_D(KConfig);
    if (d->setLocale(locale)) {
        reparseConfiguration();
        return true;
    }
    return false;
}

void KConfig::setReadDefaults(bool b)
{
    Q_D(KConfig);
    d->bReadDefaults = b;
}

bool KConfig::readDefaults() const
{
    Q_D(const KConfig);
    return d->bReadDefaults;
}

bool KConfig::isImmutable() const
{
    Q_D(const KConfig);
    return d->bFileImmutable;
}

bool KConfig::isGroupImmutableImpl(const QByteArray& aGroup) const
{
    Q_D(const KConfig);
    return isImmutable() || d->entryMap.getEntryOption(aGroup, 0, 0, KEntryMap::EntryImmutable);
}

void KConfig::setForceGlobal(bool b)
{
    Q_D(KConfig);
    d->bForceGlobal = b;
}

bool KConfig::forceGlobal() const
{
    Q_D(const KConfig);
    return d->bForceGlobal;
}

KConfigGroup KConfig::groupImpl(const QByteArray &group)
{
    return KConfigGroup(this, group.constData());
}

const KConfigGroup KConfig::groupImpl(const QByteArray &group) const
{
    return KConfigGroup(this, group.constData());
}

KEntryMap::EntryOptions convertToOptions(KConfig::WriteConfigFlags flags)
{
    KEntryMap::EntryOptions options=0;

    if (flags&KConfig::Persistent)
        options |= KEntryMap::EntryDirty;
    if (flags&KConfig::Global)
        options |= KEntryMap::EntryGlobal;
    if (flags&KConfig::Localized)
        options |= KEntryMap::EntryLocalized;
    return options;
}

void KConfig::deleteGroupImpl(const QByteArray &aGroup, WriteConfigFlags flags)
{
    Q_D(KConfig);
    KEntryMap::EntryOptions options = convertToOptions(flags)|KEntryMap::EntryDeleted;

    QByteArray theGroup = aGroup + '\x1d';
    QSet<QByteArray> groups;
    groups << aGroup;

    for (KEntryMap::ConstIterator entryMapIt( d->entryMap.constBegin() ); entryMapIt != d->entryMap.constEnd(); ++entryMapIt) {
        if (entryMapIt.key().mKey.isNull() && entryMapIt.key().mGroup.startsWith(theGroup)) {
            groups << entryMapIt.key().mGroup;
        }
    }

    foreach (const QByteArray& group, groups) {
        const QStringList keys = keyList(QString::fromUtf8(group));
        foreach (const QString& _key, keys) {
            const QByteArray &key = _key.toUtf8();
            if (d->canWriteEntry(group, key.constData())) {
                d->entryMap.setEntry(group, key, QByteArray(), options);
                d->bDirty = true;
            }
        }
    }
}

bool KConfig::isConfigWritable(bool warnUser)
{
    Q_D(KConfig);
    bool allWritable = (d->mBackend.isNull()? false: d->mBackend->isWritable());

    if (warnUser && !allWritable) {
        QString errorMsg;
        if (!d->mBackend.isNull()) // TODO how can be it be null? Set errorMsg appropriately
            errorMsg = d->mBackend->nonWritableErrorMessage();

        // Note: We don't ask the user if we should not ask this question again because we can't save the answer.
        errorMsg += i18n("Please contact your system administrator.");
        QString cmdToExec = KStandardDirs::findExe(QString("kdialog"));
        if (!cmdToExec.isEmpty() && componentData().isValid())
        {
            QProcess::execute(cmdToExec,QStringList() << "--title" << componentData().componentName()
                    << "--msgbox" << errorMsg);
        }
    }

    d->configState = allWritable ?  ReadWrite : ReadOnly; // update the read/write status

    return allWritable;
}

bool KConfig::hasGroupImpl(const QByteArray& aGroup) const
{
    Q_D(const KConfig);

    if (d->entryMap.hasEntry(aGroup)) return true;

    QByteArray subGroupMarker = aGroup + '\x1d';
    // Because this group could have only subgroups but no entries we have to
    // search for group markers
    for (KEntryMap::ConstIterator entryMapIt( d->entryMap.constBegin() ); entryMapIt != d->entryMap.constEnd(); ++entryMapIt) {
        if (entryMapIt.key().mGroup.startsWith(subGroupMarker)) {
            return true;
        }
    }
    return false;
}

bool KConfigPrivate::canWriteEntry(const QByteArray& group, const char* key, bool isDefault) const
{
    if (bFileImmutable ||
        entryMap.getEntryOption(group, key, KEntryMap::SearchLocalized, KEntryMap::EntryImmutable))
        return isDefault;
    return true;
}

void KConfigPrivate::putData( const QByteArray& group, const char* key,
                      const QByteArray& value, KConfigBase::WriteConfigFlags flags, bool expand)
{
    KEntryMap::EntryOptions options = convertToOptions(flags);

    if (bForceGlobal)
        options |= KEntryMap::EntryGlobal;
    if (expand)
        options |= KEntryMap::EntryExpansion;

    if (value.isNull()) // deleting entry
        options |= KEntryMap::EntryDeleted;

    bool dirtied = entryMap.setEntry(group, key, value, options);
    if (dirtied && (flags & KConfigBase::Persistent))
        bDirty = true;
}

QByteArray KConfigPrivate::lookupData(const QByteArray& group, const char* key,
                                      KEntryMap::SearchFlags flags) const
{
    if (bReadDefaults)
        flags |= KEntryMap::SearchDefaults;
    const KEntryMapConstIterator it = entryMap.findEntry(group, key, flags);
    if (it == entryMap.constEnd())
        return QByteArray();
    return it->mValue;
}

QString KConfigPrivate::lookupData(const QByteArray& group, const char* key,
                                   KEntryMap::SearchFlags flags, bool *expand) const
{
    if (bReadDefaults)
        flags |= KEntryMap::SearchDefaults;
    return entryMap.getEntry(group, key, QString(), flags, expand);
}

void KConfig::virtual_hook(int /*id*/, void* /*data*/)
{
	/* nothing */
}

