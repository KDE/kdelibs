/**********************************************************************************
 *   Copyright (C) 2007 by Carlo Segato <brandon.ml@gmail.com>                    *
 *   Copyright (C) 2008 Montel Laurent <montel@kde.org>                           *
 *                                                                                *
 *   This library is free software; you can redistribute it and/or                *
 *   modify it under the terms of the GNU Lesser General Public                   *
 *   License as published by the Free Software Foundation; either                 *
 *   version 2.1 of the License, or (at your option) any later version.           *
 *                                                                                *
 *   This library is distributed in the hope that it will be useful,              *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of               *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU            *
 *   Lesser General Public License for more details.                              *
 *                                                                                *
 *   You should have received a copy of the GNU Lesser General Public             *
 *   License along with this library.  If not, see <http://www.gnu.org/licenses/>.*
 *                                                                                *
 **********************************************************************************/

#include "kemoticons.h"
#include "kemoticonsprovider.h"

#include <QFile>
#include <QDir>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QDebug>
#include <QFileSystemWatcher>

#include <kpluginloader.h>
#include <kconfiggroup.h>
#include <ktar.h>
#include <kzip.h>

class KEmoticonsPrivate
{
public:
    KEmoticonsPrivate(KEmoticons *parent);
    ~KEmoticonsPrivate();
    void loadServiceList();
    KEmoticonsProvider *loadProvider(const KService::Ptr &service);
    KEmoticonsTheme loadTheme(const QString &name);

    QList<KService::Ptr> m_loaded;
    QHash<QString, KEmoticonsTheme> m_themes;
    QFileSystemWatcher m_fileWatcher;
    KEmoticons *q;

    //private slots
    void changeTheme(const QString &path);
};

KEmoticonsPrivate::KEmoticonsPrivate(KEmoticons *parent)
        : q(parent)
{
}

KEmoticonsPrivate::~KEmoticonsPrivate()
{
}

bool priorityLessThan(const KService::Ptr &s1, const KService::Ptr &s2)
{
    return (s1->property("X-KDE-Priority").toInt() > s2->property("X-KDE-Priority").toInt());
}

void KEmoticonsPrivate::loadServiceList()
{
    QString constraint("(exist Library)");
    m_loaded = KServiceTypeTrader::self()->query("KEmoticons", constraint);
    qSort(m_loaded.begin(), m_loaded.end(), priorityLessThan);
}

KEmoticonsProvider *KEmoticonsPrivate::loadProvider(const KService::Ptr &service)
{
    KPluginFactory *factory = KPluginLoader(service->library()).factory();
    if (!factory) {
        qWarning() << "Invalid plugin factory for" << service->library();
        return 0;
    }
    KEmoticonsProvider *provider = factory->create<KEmoticonsProvider>(0);
    return provider;
}

void KEmoticonsPrivate::changeTheme(const QString &path)
{
    QFileInfo info(path);
    QString name = info.dir().dirName();

    if (m_themes.contains(name)) {
        loadTheme(name);
    }
}

KEmoticonsTheme KEmoticonsPrivate::loadTheme(const QString &name)
{
    const int numberOfTheme = m_loaded.size();
    for (int i = 0; i < numberOfTheme; ++i) {
        const QString fName = m_loaded.at(i)->property("X-KDE-EmoticonsFileName").toString();
        const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "emoticons/" + name + '/' + fName);

        if (QFile::exists(path)) {
            KEmoticonsProvider *provider = loadProvider(m_loaded.at(i));
            KEmoticonsTheme theme(provider);
            theme.loadTheme(path);
            m_themes.insert(name, theme);
            m_fileWatcher.addPath(path);
            return theme;
        }
    }
    return KEmoticonsTheme();
}

KEmoticons::KEmoticons()
        : d(new KEmoticonsPrivate(this))
{
    d->loadServiceList();
    connect(&d->m_fileWatcher, SIGNAL(fileChanged(QString)), this, SLOT(changeTheme(QString)));
}

KEmoticons::~KEmoticons()
{
}

KEmoticonsTheme KEmoticons::theme() const
{
    return theme(currentThemeName());
}

KEmoticonsTheme KEmoticons::theme(const QString &name) const
{
    if (d->m_themes.contains(name)) {
        return d->m_themes.value(name);
    }

    return d->loadTheme(name);
}

QString KEmoticons::currentThemeName()
{
    KConfigGroup config(KSharedConfig::openConfig("kdeglobals"), "Emoticons");
    QString name = config.readEntry("emoticonsTheme", "kde4");
    return name;
}

QStringList KEmoticons::themeList()
{
    QStringList ls;
    const QStringList themeDirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "emoticons", QStandardPaths::LocateDirectory);

    for (int i = 0; i < themeDirs.count(); ++i) {
        QDir themeQDir(themeDirs[i]);
        themeQDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        themeQDir.setSorting(QDir::Name);
        ls << themeQDir.entryList();
    }
    return ls;
}

void KEmoticons::setTheme(const KEmoticonsTheme &theme)
{
    setTheme(theme.themeName());
}

void KEmoticons::setTheme(const QString &theme)
{
    KConfigGroup config(KSharedConfig::openConfig("kdeglobals"), "Emoticons");
    config.writeEntry("emoticonsTheme", theme);
    config.sync();
}

KEmoticonsTheme KEmoticons::newTheme(const QString &name, const KService::Ptr &service)
{
    KEmoticonsProvider *provider = d->loadProvider(service);
    KEmoticonsTheme theme(provider);
    theme.setThemeName(name);

    theme.createNew();

    return theme;
}

QStringList KEmoticons::installTheme(const QString &archiveName)
{
    QStringList foundThemes;
    KArchiveEntry *currentEntry = 0L;
    KArchiveDirectory* currentDir = 0L;
    KArchive *archive = 0L;

    QString localThemesDir( QStandardPaths::writableLocation( QStandardPaths::GenericDataLocation ) + "/emoticons" );

    if (localThemesDir.isEmpty()) {
        qCritical() << "Could not find a suitable place in which to install the emoticon theme";
        return QStringList();
    }

    QMimeDatabase db;
    const QString currentBundleMimeType = db.mimeTypeForFile(archiveName).name();

    if (currentBundleMimeType == "application/zip" ||
            currentBundleMimeType == "application/x-zip" ||
            currentBundleMimeType == "application/x-zip-compressed") {
        archive = new KZip(archiveName);
    } else if (currentBundleMimeType == "application/x-compressed-tar" ||
               currentBundleMimeType == "application/x-bzip-compressed-tar" ||
               currentBundleMimeType == "application/x-lzma-compressed-tar" ||
               currentBundleMimeType == "application/x-xz-compressed-tar" ||
               currentBundleMimeType == "application/x-gzip" ||
               currentBundleMimeType == "application/x-bzip" ||
               currentBundleMimeType == "application/x-lzma" ||
	       currentBundleMimeType == "application/x-xz") {
        archive = new KTar(archiveName);
    } else if (archiveName.endsWith(QLatin1String("jisp")) || archiveName.endsWith(QLatin1String("zip"))) {
        archive = new KZip(archiveName);
    } else {
        archive = new KTar(archiveName);
    }

    if (!archive || !archive->open(QIODevice::ReadOnly)) {
        qCritical() << "Could not open" << archiveName << "for unpacking";
        delete archive;
        return QStringList();
    }

    const KArchiveDirectory* rootDir = archive->directory();

    // iterate all the dirs looking for an emoticons.xml file
    const QStringList entries = rootDir->entries();
    for (QStringList::ConstIterator it = entries.begin(); it != entries.end(); ++it) {
        currentEntry = const_cast<KArchiveEntry*>(rootDir->entry(*it));

        if (currentEntry->isDirectory()) {
            currentDir = dynamic_cast<KArchiveDirectory*>(currentEntry);

            for (int i = 0; i < d->m_loaded.size(); ++i) {
                QString fName = d->m_loaded.at(i)->property("X-KDE-EmoticonsFileName").toString();

                if (currentDir && currentDir->entry(fName) != NULL) {
                    foundThemes.append(currentDir->name());
                }
            }
        }
    }

    if (foundThemes.isEmpty()) {
        qCritical() << "The file" << archiveName << "is not a valid emoticon theme archive";
        archive->close();
        delete archive;
        return QStringList();
    }

    for (int themeIndex = 0; themeIndex < foundThemes.size(); ++themeIndex) {
        const QString &theme = foundThemes[themeIndex];

        currentEntry = const_cast<KArchiveEntry *>(rootDir->entry(theme));
        if (currentEntry == 0) {
            // qDebug() << "couldn't get next archive entry";
            continue;
        }

        if (currentEntry->isDirectory()) {
            currentDir = dynamic_cast<KArchiveDirectory*>(currentEntry);

            if (currentDir == 0) {
                // qDebug() << "couldn't cast archive entry to KArchiveDirectory";
                continue;
            }

            currentDir->copyTo(localThemesDir + theme);
        }
    }

    archive->close();
    delete archive;

    return foundThemes;
}

void KEmoticons::setParseMode(KEmoticonsTheme::ParseMode mode)
{
    KConfigGroup config(KSharedConfig::openConfig("kdeglobals"), "Emoticons");
    config.writeEntry("parseMode", int(mode));
    config.sync();
}

KEmoticonsTheme::ParseMode KEmoticons::parseMode()
{
    KConfigGroup config(KSharedConfig::openConfig("kdeglobals"), "Emoticons");
    return (KEmoticonsTheme::ParseMode) config.readEntry("parseMode", int(KEmoticonsTheme::RelaxedParse));
}

#include "moc_kemoticons.cpp"

// kate: space-indent on; indent-width 4; replace-tabs on;
