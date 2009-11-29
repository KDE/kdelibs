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

#include <kpluginloader.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kconfiggroup.h>
#include <ktar.h>
#include <kzip.h>
#include <kmimetype.h>
#include <kdirwatch.h>

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
    KDirWatch *m_dirwatch;
    KEmoticons *q;

    //private slots
    void themeChanged(const QString &path);
};

KEmoticonsPrivate::KEmoticonsPrivate(KEmoticons *parent)
        : q(parent)
{
}

KEmoticonsPrivate::~KEmoticonsPrivate()
{
    delete m_dirwatch;
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
        kWarning() << "Invalid plugin factory for" << service->library();
        return 0;
    }
    KEmoticonsProvider *provider = factory->create<KEmoticonsProvider>(0);
    return provider;
}

void KEmoticonsPrivate::themeChanged(const QString &path)
{
    QFileInfo info(path);
    QString name = info.dir().dirName();

    if (m_themes.contains(name)) {
        loadTheme(name);
    }
}

KEmoticonsTheme KEmoticonsPrivate::loadTheme(const QString &name)
{
    for (int i = 0; i < m_loaded.size(); ++i) {
        QString fName = m_loaded.at(i)->property("X-KDE-EmoticonsFileName").toString();
        QString path = KGlobal::dirs()->findResource("emoticons", name + '/' + fName);

        if (QFile::exists(path)) {
            KEmoticonsProvider *provider = loadProvider(m_loaded.at(i));
            KEmoticonsTheme theme(provider);
            theme.loadTheme(path);
            m_themes.insert(name, theme);

            if (!m_dirwatch->contains(path)) {
                m_dirwatch->addFile(path);
            }
            return theme;
        }
    }
    return KEmoticonsTheme();
}

KEmoticons::KEmoticons()
        : d(new KEmoticonsPrivate(this))
{
    d->loadServiceList();
    d->m_dirwatch = new KDirWatch;
    connect(d->m_dirwatch, SIGNAL(dirty(const QString&)), this, SLOT(themeChanged(const QString&)));
}

KEmoticons::~KEmoticons()
{
    delete d;
}

KEmoticonsTheme KEmoticons::theme()
{
    return theme(currentThemeName());
}

KEmoticonsTheme KEmoticons::theme(const QString &name)
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
    const QStringList themeDirs = KGlobal::dirs()->findDirs("emoticons", "");

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

    QString localThemesDir(KStandardDirs::locateLocal("emoticons", QString()));

    if (localThemesDir.isEmpty()) {
        kError() << "Could not find a suitable place in which to install the emoticon theme";
        return QStringList();
    }

    QString currentBundleMimeType = KMimeType::findByPath(archiveName, 0, false)->name();

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
        kError() << "Could not open" << archiveName << "for unpacking";
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
        kError() << "The file" << archiveName << "is not a valid emoticon theme archive";
        archive->close();
        delete archive;
        return QStringList();
    }

    for (int themeIndex = 0; themeIndex < foundThemes.size(); ++themeIndex) {
        const QString &theme = foundThemes[themeIndex];

        currentEntry = const_cast<KArchiveEntry *>(rootDir->entry(theme));
        if (currentEntry == 0) {
            kDebug() << "couldn't get next archive entry";
            continue;
        }

        if (currentEntry->isDirectory()) {
            currentDir = dynamic_cast<KArchiveDirectory*>(currentEntry);

            if (currentDir == 0) {
                kDebug() << "couldn't cast archive entry to KArchiveDirectory";
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

#include "kemoticons.moc"

// kate: space-indent on; indent-width 4; replace-tabs on;
