/*  This file is part of the KDE libraries
 *  Copyright (C) 2006-2007, 2010 David Faure <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
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
 */

#include "kmimetyperepository_p.h"
#include <kdebug.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include "kmimetype.h"
#include <kdeversion.h> // KDE_MAKE_VERSION
#include <qstandardpaths.h>
#include <QFile>
#include <QProcess>

extern int servicesDebugArea();

class KMimeTypeRepositorySingleton
{
public:
    KMimeTypeRepository instance;
};

Q_GLOBAL_STATIC(KMimeTypeRepositorySingleton, s_self)

KMimeTypeRepository * KMimeTypeRepository::self()
{
    return &s_self()->instance;
}

KMimeTypeRepository::KMimeTypeRepository()
    : m_useFavIcons(true),
      m_useFavIconsChecked(false),
      m_sharedMimeInfoVersion(0),
      m_mutex(QReadWriteLock::Recursive)
{
}

KMimeTypeRepository::~KMimeTypeRepository()
{
}

bool KMimeTypeRepository::matchFileName( const QString &filename, const QString &pattern )
{
    const int pattern_len = pattern.length();
    if (!pattern_len)
        return false;
    const int len = filename.length();

    const int starCount = pattern.count(QLatin1Char('*'));

    // Patterns like "*~", "*.extension"
    if (pattern[0] == QLatin1Char('*')  && pattern.indexOf(QLatin1Char('[')) == -1 && starCount == 1)
    {
        if ( len + 1 < pattern_len ) return false;

        const QChar *c1 = pattern.unicode() + pattern_len - 1;
        const QChar *c2 = filename.unicode() + len - 1;
        int cnt = 1;
        while (cnt < pattern_len && *c1-- == *c2--)
            ++cnt;
        return cnt == pattern_len;
    }

    // Patterns like "README*" (well this is currently the only one like that...)
    if (starCount == 1 && pattern[pattern_len - 1] == QLatin1Char('*')) {
        if ( len + 1 < pattern_len ) return false;
        if (pattern[0] == QLatin1Char('*'))
            return filename.indexOf(pattern.mid(1, pattern_len - 2)) != -1;

        const QChar *c1 = pattern.unicode();
        const QChar *c2 = filename.unicode();
        int cnt = 1;
        while (cnt < pattern_len && *c1++ == *c2++)
           ++cnt;
        return cnt == pattern_len;
    }

    // Names without any wildcards like "README"
    if (pattern.indexOf(QLatin1Char('[')) == -1 && starCount == 0 && pattern.indexOf(QLatin1Char('?')))
        return (pattern == filename);

    // Other (quite rare) patterns, like "*.anim[1-9j]": use slow but correct method
    QRegExp rx(pattern);
    rx.setPatternSyntax(QRegExp::Wildcard);
    return rx.exactMatch(filename);
}

QStringList KMimeTypeRepository::patternsForMimetype(const QString& mimeType)
{
    //QWriteLocker lock(&m_mutex);
    QMimeType mime = m_mimeDb.mimeTypeForName(mimeType);
    return mime.globPatterns();
}

KMimeType::Ptr KMimeTypeRepository::defaultMimeTypePtr()
{
    QWriteLocker lock(&m_mutex);
    if (!m_defaultMimeType) {
        // Try to find the default type
        QMimeType qmime = m_mimeDb.mimeTypeForName(KMimeType::defaultMimeType());
        KMimeType::Ptr mime(new KMimeType(qmime));
        m_defaultMimeType = mime;
    }
    return m_defaultMimeType;
}

bool KMimeTypeRepository::useFavIcons()
{
    // this method will be called quite often, so better not read the config
    // again and again.
    m_mutex.lockForWrite();
    if (!m_useFavIconsChecked) {
        m_useFavIconsChecked = true;
        KConfigGroup cg( KSharedConfig::openConfig(), "HTML Settings" );
        m_useFavIcons = cg.readEntry("EnableFavicon", true);
    }
    m_mutex.unlock();
    return m_useFavIcons;
}

static void addPlatformSpecificPkgConfigPath(QStringList& paths)
{
#if defined (Q_OS_FREEBSD)
    paths << QLatin1String("/usr/local/libdata/pkgconfig"); // FreeBSD
#elif defined(Q_OS_OPENBSD) || defined(Q_OS_NETBSD) || defined(Q_OS_SOLARIS)
    paths << QLatin1String("/usr/local/lib/pkgconfig"); // {Net,Open}BSD/OpenSolaris
#elif defined (Q_OS_UNIX)
    paths << QLatin1String("/usr/share/pkgconfig"); // Linux and all other unix
#endif
}

static int mimeDataBaseVersion()
{
    // TODO: Remove the #idef'ed code below once the issue is fixed either
    // in QProcess or the shared-mime-info utility provides its version number.
#ifdef Q_OS_UNIX
    // Try to read the version number from the shared-mime-info.pc file
    QStringList paths;
    const QByteArray pkgConfigPath = qgetenv("PKG_CONFIG_PATH");
    if (!pkgConfigPath.isEmpty()) {
        paths << QFile::decodeName(pkgConfigPath).split(QLatin1Char(':'), QString::SkipEmptyParts);
    }

    // Add platform specific hard-coded default paths to the list...
    addPlatformSpecificPkgConfigPath(paths);

    Q_FOREACH(const QString& path, paths) {
        const QString fileName = path + QLatin1String("/shared-mime-info.pc");
        if (!QFile::exists(fileName)) {
            continue;
        }

        QFile file (fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            break;
        }

        while (!file.atEnd()) {
            const QByteArray line = file.readLine().simplified();
            if (!line.startsWith("Version")) { // krazy:exclude=strings
                continue;
            }
            QRegExp versionRe(QString::fromLatin1("Version: (\\d+)\\.(\\d+)(\\.(\\d+))?"));
            if (versionRe.indexIn(QString::fromLocal8Bit(line)) > -1) {
                return KDE_MAKE_VERSION(versionRe.cap(1).toInt(), versionRe.cap(2).toInt(), versionRe.cap(4).toInt());
            }
        }
    }
#endif

    // Execute "update-mime-database -v" to determine version number.
    // NOTE: On *nix, the code below is known to cause freezes/hangs in apps
    // that block signals. See https://bugs.kde.org/show_bug.cgi?id=260719.
    const QString umd = QStandardPaths::findExecutable(QString::fromLatin1("update-mime-database"));
    if (umd.isEmpty()) {
        kWarning() << "update-mime-database not found!";
        return -1;
    }

    QProcess smi;
    smi.start(umd, QStringList() << QString::fromLatin1("-v"));
    smi.waitForStarted();
    smi.waitForFinished();
    const QString out = QString::fromLocal8Bit(smi.readAllStandardError());
    QRegExp versionRe(QString::fromLatin1("update-mime-database \\(shared-mime-info\\) (\\d+)\\.(\\d+)(\\.(\\d+))?"));
    if (versionRe.indexIn(out) > -1) {
        return KDE_MAKE_VERSION(versionRe.cap(1).toInt(), versionRe.cap(2).toInt(), versionRe.cap(4).toInt());
    }

    kWarning() << "Unexpected version scheme from update-mime-database -v: got" << out;
    return -1;
}

int KMimeTypeRepository::sharedMimeInfoVersion()
{
    m_mutex.lockForWrite();
    if (m_sharedMimeInfoVersion == 0)
        m_sharedMimeInfoVersion = mimeDataBaseVersion();
    m_mutex.unlock();
    return m_sharedMimeInfoVersion;
}
