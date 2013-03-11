/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *                2000-2007 David Faure <faure@kde.org>
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

#include "kmimetype.h"
#include "kmimetype_p.h"
#include "kmimetypefactory.h"
#include "kmimetyperepository_p.h"

#include <kdebug.h>
#include <kde_file.h> // KDE::stat
#include <kdeversion.h> // KDE_MAKE_VERSION
#include <klocale.h>
#include <kprotocolinfo.h>
#include <kprotocolinfofactory.h>
#include <kstandarddirs.h>
#include <kurl.h>

#include <QtCore/QFile>
#include <QtDBus/QtDBus>
#include <QtCore/QHash>
#include <QBuffer>

extern int servicesDebugArea();

template class KSharedPtr<KMimeType>;

KMimeType::Ptr KMimeType::defaultMimeTypePtr()
{
    return KMimeTypeRepository::self()->defaultMimeTypePtr();
}

bool KMimeType::isDefault() const
{
    return name() == defaultMimeType();
}

void KMimeType::checkEssentialMimeTypes()
{
    KMimeTypeRepository::self()->checkEssentialMimeTypes();
}

KMimeType::Ptr KMimeType::mimeType(const QString& name, FindByNameOption options)
{
    return KMimeTypeRepository::self()->findMimeTypeByName(name, options);
}

KMimeType::List KMimeType::allMimeTypes()
{
    // This could be done faster...
    KMimeType::List lst;
    Q_FOREACH(const QString& mimeType, KMimeTypeFactory::self()->allMimeTypes()) {
        if (!mimeType.startsWith(QLatin1String("x-scheme-handler")))
            lst.append(KMimeType::mimeType(mimeType));
    }
    return lst;
}

bool KMimeType::isBufferBinaryData(const QByteArray& data)
{
    // Check the first 32 bytes (see shared-mime spec)
    const char* p = data.data();
    const int end = qMin(32, data.size());
    for (int i = 0; i < end; ++i) {
        if ((unsigned char)(p[i]) < 32 && p[i] != 9 && p[i] != 10 && p[i] != 13) // ASCII control character
            return true;
    }
    return false;
}

static KMimeType::Ptr findFromMode( const QString& path /*only used if is_local_file*/,
                                    mode_t mode /*0 if unknown*/,
                                    bool is_local_file )
{
    if ( is_local_file && (mode == 0 || mode == (mode_t)-1) ) {
        KDE_struct_stat buff;
        if ( KDE::stat( path, &buff ) != -1 )
            mode = buff.st_mode;
    }

    if ( S_ISDIR( mode ) ) {
        // KDE4 TODO: use an overlay instead
#if 0
        // Special hack for local files. We want to see whether we
        // are allowed to enter the directory
        if ( is_local_file )
        {
            if ( KDE::access( path, R_OK ) == -1 )
                return KMimeType::mimeType( "inode/directory-locked" );
        }
#endif
        return KMimeType::mimeType( QLatin1String("inode/directory") );
    }
    if ( S_ISCHR( mode ) )
        return KMimeType::mimeType( QLatin1String("inode/chardevice") );
    if ( S_ISBLK( mode ) )
        return KMimeType::mimeType( QLatin1String("inode/blockdevice") );
    if ( S_ISFIFO( mode ) )
        return KMimeType::mimeType( QLatin1String("inode/fifo") );
    if ( S_ISSOCK( mode ) )
        return KMimeType::mimeType( QLatin1String("inode/socket") );
#ifdef Q_OS_WIN
    // FIXME: distinguish between mounted & unmounted
    int size = path.size();
    if ( size == 2 || size == 3 ) {
    //GetDriveTypeW is not defined in wince
#ifndef _WIN32_WCE
        unsigned int type = GetDriveTypeW( (LPCWSTR) path.utf16() );
        switch( type ) {
            case DRIVE_REMOVABLE:
                return KMimeType::mimeType( QLatin1String("media/floppy_mounted") );
            case DRIVE_FIXED:
                return KMimeType::mimeType( QLatin1String("media/hdd_mounted") );
            case DRIVE_REMOTE:
                return KMimeType::mimeType( QLatin1String("media/smb_mounted") );
            case DRIVE_CDROM:
                return KMimeType::mimeType( QLatin1String("media/cdrom_mounted") );
            case DRIVE_RAMDISK:
                return KMimeType::mimeType( QLatin1String("media/hdd_mounted") );
            default:
                break;
        };
#else
        return KMimeType::mimeType( QLatin1String("media/hdd_mounted") );
#endif
    }
#endif
    // remote executable file? stop here (otherwise findFromContent can do that better for local files)
    if ( !is_local_file && S_ISREG( mode ) && ( mode & ( S_IXUSR | S_IXGRP | S_IXOTH ) ) )
        return KMimeType::mimeType( QLatin1String("application/x-executable") );

    return KMimeType::Ptr();
}

/*

As agreed on the XDG list (and unlike the current shared-mime spec):

Glob-matching should prefer derived mimetype over base mimetype, and longer matches
over shorter ones. However if two globs of the same length match the file, and the two
matches are not related in the inheritance tree, then we have a "glob conflict", which
will be resolved below.

If only one glob matches, use that

If no glob matches, sniff and use that

If several globs matches, and sniffing gives a result we do:
  if sniffed prio >= 80, use sniffed type
  for glob_match in glob_matches:
     if glob_match is subclass or equal to sniffed_type, use glob_match

If several globs matches, and sniffing fails, or doesn't help:
  fall back to the first glob match

This algorithm only sniffs when there is some uncertainty with the
extension matching (thus, it's usable for a file manager).

Note: in KDE we want the file views to sniff in a delayed manner.
So there's also a fast mode which is:
 if no glob matches, or if more than one glob matches, use default mimetype and mark as "can be refined".

*/

KMimeType::Ptr KMimeType::findByUrlHelper( const KUrl& _url, mode_t mode,
                                           bool is_local_file,
                                           QIODevice* device,
                                           int* accuracy )
{
    checkEssentialMimeTypes();
    const QString path = is_local_file ? _url.toLocalFile() : _url.path();

    if (accuracy)
        *accuracy = 100;

    // Look at mode first
    KMimeType::Ptr mimeFromMode = findFromMode( path, mode, is_local_file );
    if (mimeFromMode)
        return mimeFromMode;

    // First try to find out by looking at the filename (if there's one)
    const QString fileName( _url.fileName() );
    QStringList mimeList;
    if ( !fileName.isEmpty() && !path.endsWith( QLatin1Char('/') ) ) {
        // and if we can trust it (e.g. don't trust *.pl over HTTP, could be anything)
        if ( is_local_file || _url.hasSubUrl() || // Explicitly trust suburls
             KProtocolInfo::determineMimetypeFromExtension( _url.protocol() ) ) {
            mimeList = KMimeTypeRepository::self()->findFromFileName( fileName );
            // Found one glob match exactly: OK, use that.
            // We disambiguate multiple glob matches by sniffing, below.
            if ( mimeList.count() == 1 ) {
                const QString selectedMime = mimeList.at(0);
                KMimeType::Ptr mime = mimeType(selectedMime);
                if (!mime) {
                    // #265188 - this can happen when an old globs file is lying around after
                    // the packages xml file was removed.
                    kWarning() << "Glob file refers to" << selectedMime << "but this mimetype does not exist!";
                    mimeList.clear();
                } else {
                    return mime;
                }
            }
        }
    }

    if ( device && !device->isOpen() ) {
        if ( !device->open(QIODevice::ReadOnly) ) {
            device = 0;
        }
    }

    // Try the magic matches (if we can read the data)
    QByteArray beginning;
    if ( device ) {
        int magicAccuracy;
        KMimeType::Ptr mime = KMimeTypeRepository::self()->findFromContent(device, &magicAccuracy, beginning);
        // mime can't be 0, except in case of install problems.
        // However we get magicAccuracy==0 for octet-stream, i.e. no magic match found.
        //kDebug(servicesDebugArea()) << "findFromContent said" << (mime?mime->name():QString()) << "with accuracy" << magicAccuracy;
        if (mime && magicAccuracy > 0) {

            // Disambiguate conflicting extensions (if magic found something and the magicrule was <80)
            if (magicAccuracy < 80 && !mimeList.isEmpty()) {
                // "for glob_match in glob_matches:"
                // "if glob_match is subclass or equal to sniffed_type, use glob_match"
                const QString sniffedMime = mime->name();
                foreach(const QString &m, mimeList) {
                    KMimeType::Ptr mimeFromPattern = KMimeType::mimeType(m);
                    //kDebug(servicesDebugArea()) << "sniffedMime=" << sniffedMime << "mimeFromPattern=" << mimeFromPattern->name();
                    if (mimeFromPattern && mimeFromPattern->is(sniffedMime)) {
                        // We have magic + pattern pointing to this, so it's a pretty good match
                        if (accuracy)
                            *accuracy = 100;
                        return mimeFromPattern;
                    }
                }
            }

            if (accuracy)
                *accuracy = magicAccuracy;
            return mime;
        }
    }

    // Not a local file, or no magic allowed, or magic found nothing

    // Maybe we had multiple matches from globs?
    if (!mimeList.isEmpty()) {
        if (accuracy)
            *accuracy = 20;
        // We have to pick one...
        // At least make this deterministic
        qSort(mimeList.begin(), mimeList.end());
        Q_FOREACH(const QString& mimeName, mimeList) {
            KMimeType::Ptr mime = mimeType(mimeName);
            if (!mime)
                kWarning() << "Glob file refers to" << mimeName << "but this mimetype does not exist!";
            else
                return mime;
        }
    }

    // Find a fallback from the protocol
    if (accuracy)
        *accuracy = 10;
    // ## this breaks with proxying; find a way to move proxying info to kdecore's kprotocolinfo?
    // ## or hardcode the only case of proxying that we ever had? (ftp-over-http)
    KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol( _url.protocol() );
    QString def;
    if (prot)
        def = prot->defaultMimeType();
    if ( !def.isEmpty() && def != defaultMimeType() ) {
        // The protocol says it always returns a given mimetype (e.g. text/html for "man:")
        KMimeType::Ptr mime = mimeType( def );
        if (mime)
            return mime;
    }
    if ( path.endsWith( QLatin1Char('/') ) || path.isEmpty() ) {
        // We have no filename at all. Maybe the protocol has a setting for
        // which mimetype this means (e.g. directory).
        // For HTTP (def==defaultMimeType()) we don't assume anything,
        // because of redirections (e.g. freshmeat downloads).
        if ( def.isEmpty() ) {
            // Assume inode/directory, if the protocol supports listing.
            KProtocolInfo::Ptr prot = KProtocolInfoFactory::self()->findProtocol( _url.protocol() );
            if ( prot && prot->supportsListing() ) {
                KMimeType::Ptr mime = mimeType( QLatin1String("inode/directory") );
                if (mime) { // only 0 if no mimetypes installed
                    return mime;
                }
            } else
                return defaultMimeTypePtr(); // == 'no idea', e.g. for "data:,foo/"
        }
    }

    if (accuracy)
        *accuracy = 0;
    return defaultMimeTypePtr();
}

KMimeType::Ptr KMimeType::findByUrl( const KUrl& url, mode_t mode,
                                     bool is_local_file, bool fast_mode,
                                     int *accuracy )
{
    if ( !is_local_file && url.isLocalFile() )
        is_local_file = true;
    if (is_local_file && !fast_mode) {
        QFile file(url.toLocalFile());
        return findByUrlHelper(url, mode, is_local_file, &file, accuracy);
    }
    return findByUrlHelper(url, mode, is_local_file, 0, accuracy);
}

KMimeType::Ptr KMimeType::findByPath( const QString& path, mode_t mode,
                                      bool fast_mode, int* accuracy )
{
    KUrl url;
    url.setPath(path);
    return findByUrl(url, mode, true, fast_mode, accuracy);
}

KMimeType::Ptr KMimeType::findByNameAndContent( const QString& name, const QByteArray& data,
                                                mode_t mode, int* accuracy )
{
    KUrl url;
    url.setPath(name);
    QBuffer buffer(const_cast<QByteArray *>(&data));
    return findByUrlHelper(url, mode, false, &buffer, accuracy);
}

KMimeType::Ptr KMimeType::findByNameAndContent( const QString& name, QIODevice* device,
                                                mode_t mode, int* accuracy )
{
    KUrl url;
    url.setPath(name);
    return findByUrlHelper(url, mode, false, device, accuracy);
}

QString KMimeType::extractKnownExtension(const QString &fileName)
{
    QString pattern;
    KMimeTypeRepository::self()->findFromFileName( fileName, &pattern );
    return pattern;
}

KMimeType::Ptr KMimeType::findByContent( const QByteArray &data, int *accuracy )
{
    QBuffer buffer(const_cast<QByteArray *>(&data));
    buffer.open(QIODevice::ReadOnly);
    QByteArray cache;
    return KMimeTypeRepository::self()->findFromContent(&buffer, accuracy, cache);
}

KMimeType::Ptr KMimeType::findByContent( QIODevice* device, int* accuracy )
{
    QByteArray cache;
    return KMimeTypeRepository::self()->findFromContent(device, accuracy, cache);
}

KMimeType::Ptr KMimeType::findByFileContent( const QString &fileName, int *accuracy )
{
    checkEssentialMimeTypes();

    QFile device(fileName);
    // Look at mode first
    KMimeType::Ptr mimeFromMode = findFromMode( fileName, 0, true );
    if (mimeFromMode) {
        if (accuracy)
            *accuracy = 100;
        return mimeFromMode;
    }
    if (!device.open(QIODevice::ReadOnly)) {
        if (accuracy)
            *accuracy = 0;
        return KMimeType::defaultMimeTypePtr();
    }

    QByteArray cache;
    return KMimeTypeRepository::self()->findFromContent(&device, accuracy, cache);
}

bool KMimeType::isBinaryData( const QString &fileName )
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return false; // err, whatever
    const QByteArray data = file.read(32);
    return isBufferBinaryData(data);
}

KMimeType::KMimeType( KMimeTypePrivate &dd, const QString& name,
                      const QString& comment )
    : KServiceType( dd, name, comment )
{
}

KMimeType::KMimeType( const QString & fullpath, const QString& name,
                      const QString& comment )
    : KServiceType( *new KMimeTypePrivate(fullpath), name, comment )
{
}

KMimeType::KMimeType( KMimeTypePrivate &dd)
    : KServiceType(dd)
{
}

KMimeType::KMimeType( QDataStream& _str, int offset )
    : KServiceType( *new KMimeTypePrivate(_str, offset ))
{
}

void KMimeTypePrivate::save( QDataStream& _str )
{
    KServiceTypePrivate::save( _str );
    // Warning adding fields here involves a binary incompatible change - update version
    // number in ksycoca.h. Never remove fields.
    _str << m_lstPatterns << QString() << QStringList() << m_iconName;
}

QVariant KMimeTypePrivate::property( const QString& _name ) const
{
    if ( _name == QLatin1String("Patterns") )
        return QVariant( m_lstPatterns );
    if ( _name == QLatin1String("Icon") )
        return QVariant( iconName(KUrl()) );

    return KServiceTypePrivate::property( _name );
}

QStringList KMimeTypePrivate::propertyNames() const
{
    QStringList res = KServiceTypePrivate::propertyNames();
    res.append( QString::fromLatin1("Patterns") );
    res.append( QString::fromLatin1("Icon") );
    return res;
}

KMimeType::~KMimeType()
{
}

QString KMimeType::iconNameForUrl( const KUrl & _url, mode_t mode )
{
    const KMimeType::Ptr mt = findByUrl( _url, mode, _url.isLocalFile(),
                                         false /*HACK*/);
    if (!mt) {
        return QString();
    }
    static const QString& unknown = KGlobal::staticQString("unknown");
    const QString mimeTypeIcon = mt->iconName( _url );
    QString i = mimeTypeIcon;

    // if we don't find an icon, maybe we can use the one for the protocol
    if ( i == unknown || i.isEmpty() || mt->name() == defaultMimeType()
        // and for the root of the protocol (e.g. trash:/) the protocol icon has priority over the mimetype icon
        || _url.path().length() <= 1 )
    {
        i = favIconForUrl( _url ); // maybe there is a favicon?

        if ( i.isEmpty() )
            i = KProtocolInfo::icon( _url.protocol() );

        // root of protocol: if we found nothing, revert to mimeTypeIcon (which is usually "folder")
        if ( _url.path().length() <= 1 && ( i == unknown || i.isEmpty() ) )
            i = mimeTypeIcon;
    }
    return !i.isEmpty() ? i : unknown;
}

QString KMimeType::favIconForUrl( const KUrl& url )
{
    /* The kded module also caches favicons, for one week, without any way
     * to clean up the cache meanwhile.
     * On the other hand, this QHash will get cleaned up after 5000 request
     * (a selection in konsole of 80 chars generates around 500 requests)
     * or by simply restarting the application (or the whole desktop,
     * more likely, for the case of konqueror or konsole).
     */
    static QHash<QUrl, QString> iconNameCache;
    static int autoClearCache = 0;
    const QString notFound = QLatin1String("NOTFOUND");

    if (url.isLocalFile()
        || !url.protocol().startsWith(QLatin1String("http"))
        || !KMimeTypeRepository::self()->useFavIcons())
        return QString();

    QString iconNameFromCache = iconNameCache.value(url, notFound);
    if ( iconNameFromCache != notFound ) {
        if ( (++autoClearCache) < 5000 ) {
            return iconNameFromCache;
        }
        else {
            iconNameCache.clear();
            autoClearCache = 0;
        }
    }

    QDBusInterface kded( QString::fromLatin1("org.kde.kded"),
                         QString::fromLatin1("/modules/favicons"),
                         QString::fromLatin1("org.kde.FavIcon") );
    QDBusReply<QString> result = kded.call( QString::fromLatin1("iconForUrl"), url.url() );
    iconNameCache.insert(url, result.value());
    return result;              // default is QString()
}

QString KMimeType::comment( const KUrl &url) const
{
    Q_D(const KMimeType);
    return d->comment(url);
}

#ifndef KDE_NO_DEPRECATED
QString KMimeType::parentMimeType() const
{
    const QStringList parents = parentMimeTypes();
    if (!parents.isEmpty())
        return parents.first();
    return QString();
}
#endif

bool KMimeTypePrivate::inherits(const QString& mime) const
{
    QStack<QString> toCheck;
    toCheck.push(m_strName);
    while (!toCheck.isEmpty()) {
        const QString current = toCheck.pop();
        if (current == mime)
            return true;
        Q_FOREACH(const QString& parent, KMimeTypeRepository::self()->parents(current)) {
            toCheck.push(parent);
        }
    }
    return false;
}

bool KMimeType::is( const QString& mimeTypeName ) const
{
    Q_D(const KMimeType);
    if (name() == mimeTypeName)
        return true;
    const QString mime = KMimeTypeRepository::self()->canonicalName(mimeTypeName);
    return d->inherits(mime);
}

QStringList KMimeType::parentMimeTypes() const
{
    Q_D(const KMimeType);
    return KMimeTypeRepository::self()->parents(d->m_strName);
}

static void collectParentMimeTypes(const QString& mime, QStringList& allParents)
{
    QStringList parents = KMimeTypeRepository::self()->parents(mime);
    Q_FOREACH(const QString& parent, parents) {
        // I would use QSet, but since order matters I better not
        if (!allParents.contains(parent))
            allParents.append(parent);
    }
    // We want a breadth-first search, so that the least-specific parent (octet-stream) is last
    // This means iterating twice, unfortunately.
    Q_FOREACH(const QString& parent, parents) {
        collectParentMimeTypes(parent, allParents);
    }
}

QStringList KMimeType::allParentMimeTypes() const
{
    Q_D(const KMimeType);
    QStringList allParents;
    const QString canonical = KMimeTypeRepository::self()->resolveAlias(name());
    if (!canonical.isEmpty())
        allParents.append(canonical);
    collectParentMimeTypes(d->m_strName, allParents);
    return allParents;
}

QString KMimeType::defaultMimeType()
{
    static const QString & s_strDefaultMimeType =
        KGlobal::staticQString( "application/octet-stream" );
    return s_strDefaultMimeType;
}

QString KMimeType::iconName( const KUrl& url) const
{
    Q_D(const KMimeType);
    return d->iconName(url);
}

QStringList KMimeType::patterns() const
{
    Q_D(const KMimeType);
    d->ensureXmlDataLoaded();
    return d->m_lstPatterns;
}

// loads comment, icon, mainPattern, m_lstPatterns
void KMimeTypePrivate::ensureXmlDataLoaded() const
{
    if (m_xmlDataLoaded)
        return;

    m_xmlDataLoaded = true;

    const QString file = m_strName + QLatin1String(".xml");
    const QStringList mimeFiles = KGlobal::dirs()->findAllResources("xdgdata-mime", file);
    if (mimeFiles.isEmpty()) {
        kWarning() << "No file found for" << file << ", even though the file appeared in a directory listing.";
        kWarning() << "Either it was just removed, or the directory doesn't have executable permission...";
        kWarning() << KGlobal::dirs()->resourceDirs("xdgdata-mime");
        return;
    }

    QString comment;
    QString mainPattern;
    const QStringList languageList = KGlobal::locale()->languageList();
    QString preferredLanguage = languageList.first();
    QMap<QString, QString> commentsByLanguage;

    QListIterator<QString> mimeFilesIter(mimeFiles);
    mimeFilesIter.toBack();
    while (mimeFilesIter.hasPrevious()) { // global first, then local.
        const QString fullPath = mimeFilesIter.previous();
        QFile qfile(fullPath);
        if (!qfile.open(QFile::ReadOnly))
            continue;

        QXmlStreamReader xml(&qfile);
        if (xml.readNextStartElement()) {
            if (xml.name() != "mime-type") {
                continue;
            }
            const QString name = xml.attributes().value(QLatin1String("type")).toString();
            if (name.isEmpty())
                continue;
            if (name != m_strName) {
                kWarning() << "Got name" << name << "in file" << file << "expected" << m_strName;
            }

            while (xml.readNextStartElement()) {
                const QStringRef tag = xml.name();
                if (tag == "comment") {
                    QString lang = xml.attributes().value(QLatin1String("xml:lang")).toString();
                    const QString text = xml.readElementText();
                    if (lang.isEmpty()) {
                        lang = QLatin1String("en_US");
                    }
                    if (lang == preferredLanguage) {
                        comment = text;
                    } else {
                        commentsByLanguage.insert(lang, text);
                    }
                    continue; // we called readElementText, so we're at the EndElement already.
                } else if (tag == "icon") { // as written out by shared-mime-info >= 0.40
                    m_iconName = xml.attributes().value(QLatin1String("name")).toString();
                } else if (tag == "glob-deleteall") { // as written out by shared-mime-info >= 0.70
                    mainPattern.clear();
                    m_lstPatterns.clear();
                } else if (tag == "glob") { // as written out by shared-mime-info >= 0.70
                    const QString pattern = xml.attributes().value(QLatin1String("pattern")).toString();
                    if (mainPattern.isEmpty() && pattern.startsWith(QLatin1Char('*'))) {
                        mainPattern = pattern;
                    }
                    if (!m_lstPatterns.contains(pattern))
                        m_lstPatterns.append(pattern);
                }
                xml.skipCurrentElement();
            }
            if (xml.name() != "mime-type") {
                kFatal() << "Programming error in KMimeType XML loading, please create a bug report on http://bugs.kde.org and attach the file" << fullPath;
            }
        }
    }

    if (comment.isEmpty()) {
        Q_FOREACH(const QString& lang, languageList) {
            const QString comm = commentsByLanguage.value(lang);
            if (!comm.isEmpty()) {
                comment = comm;
                break;
            }
            const int pos = lang.indexOf(QLatin1Char('_'));
            if (pos != -1) {
                // "pt_BR" not found? try just "pt"
                const QString shortLang = lang.left(pos);
                const QString comm = commentsByLanguage.value(shortLang);
                if (!comm.isEmpty()) {
                    comment = comm;
                    break;
                }
            }
        }
        if (comment.isEmpty()) {
            kWarning() << "Missing <comment> field in" << file;
        }
    }
    m_strComment = comment;

    const bool globsInXml = (KMimeType::sharedMimeInfoVersion() >= KDE_MAKE_VERSION(0, 70, 0));
    if (globsInXml) {
        if (!mainPattern.isEmpty() && m_lstPatterns.first() != mainPattern) {
            // ensure it's first in the list of patterns
            m_lstPatterns.removeAll(mainPattern);
            m_lstPatterns.prepend(mainPattern);
        }
    } else {
        // Fallback: get the patterns from the globs file
        m_lstPatterns = KMimeTypeRepository::self()->patternsForMimetype(m_strName);
    }
}

QString KMimeType::userSpecifiedIconName() const
{
    Q_D(const KMimeType);
    d->ensureXmlDataLoaded();
    return d->m_iconName;
}

int KMimeType::sharedMimeInfoVersion()
{
    return KMimeTypeRepository::self()->sharedMimeInfoVersion();
}

QString KMimeType::mainExtension() const
{
    Q_D(const KMimeType);

#if 1 // HACK START - can be removed once shared-mime-info >= 0.70 is used/required.
    // The idea was: first usable pattern from m_lstPatterns.
    // But update-mime-database makes a mess of the order of the patterns,
    // because it uses a hash internally.
    static const struct { const char* mime; const char* extension; } s_hardcodedMimes[] = {
        { "text/plain", ".txt" } };
    if (d->m_lstPatterns.count() > 1) {
        const QByteArray me = name().toLatin1();
        for (uint i = 0; i < sizeof(s_hardcodedMimes)/sizeof(*s_hardcodedMimes); ++i) {
            if (me == s_hardcodedMimes[i].mime)
                return QString::fromLatin1(s_hardcodedMimes[i].extension);
        }
    }
#endif // HACK END

     Q_FOREACH(const QString& pattern, patterns()) {
        // Skip if if looks like: README or *. or *.*
        // or *.JP*G or *.JP?
        if (pattern.startsWith(QLatin1String("*.")) &&
            pattern.length() > 2 &&
            pattern.indexOf(QLatin1Char('*'), 2) < 0 && pattern.indexOf(QLatin1Char('?'), 2) < 0) {
            return pattern.mid(1);
        }
    }
    // TODO we should also look into the parent mimetype's patterns, no?
    return QString();
}

bool KMimeType::matchFileName( const QString &filename, const QString &pattern )
{
    return KMimeTypeRepository::matchFileName( filename, pattern );
}

int KMimeTypePrivate::serviceOffersOffset() const
{
    return KMimeTypeFactory::self()->serviceOffersOffset(name());
}

QString KMimeTypePrivate::iconName(const KUrl &) const
{
    static QHash<QUrl, QString> iconNameCache;
    QString iconNameFromCache = iconNameCache.value(m_strName);
    if (!iconNameFromCache.isEmpty())
        return iconNameFromCache;

    ensureXmlDataLoaded();
    QString result;
    if (!m_iconName.isEmpty()) {
        result = m_iconName;
    } else {
        // Make default icon name from the mimetype name
        // Don't store this in m_iconName, it would make the filetype editor
        // write out icon names in every local mimetype definition file.
        QString icon = m_strName;
        const int slashindex = icon.indexOf(QLatin1Char('/'));
        if (slashindex != -1) {
            icon[slashindex] = QLatin1Char('-');
        }
        result = icon;
    }
    iconNameCache.insert(m_strName, result);
    return result;
}
