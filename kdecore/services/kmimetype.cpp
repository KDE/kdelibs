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

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kde_file.h>
#include <klocale.h>
#include <kmessage.h>
#include <kprotocolinfo.h>
#include <kprotocolinfofactory.h>
#include <kstandarddirs.h>
#include <ksycoca.h>
#include <kurl.h>

#include <QtCore/QMutableSetIterator>
#include <QtCore/QCharRef>
#include <QtCore/QFile>
#include <QtDBus/QtDBus>
#include <QBuffer>

#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>

template class KSharedPtr<KMimeType>;

static KMimeType::Ptr s_pDefaultMimeType;

static void errorMissingMimeTypes( const QStringList& _types )
{
    KMessage::message( KMessage::Error, i18np( "Could not find mime type <resource>%2</resource>",
                "Could not find mime types:\n<resource>%2</resource>", _types.count(), _types.join("</resource>\n<resource>") ) );
}

void KMimeTypePrivate::loadInternal( QDataStream& _str )
{
    QString oldParentMimeTypeString;
    _str >> m_lstPatterns >> oldParentMimeTypeString >> m_parentMimeTypes;

    // kde-4.0 compatibility. Remove in kde5.
    if (!oldParentMimeTypeString.isEmpty() && m_parentMimeTypes.isEmpty())
        m_parentMimeTypes.append(oldParentMimeTypeString);
}

/**
 * This function makes sure that the default mime type exists.
 * Not file-static because it needs access to the private KMimeType constructor...
 */
void KMimeType::buildDefaultType()
{
    assert ( !s_pDefaultMimeType );
    // Try to find the default type
    KMimeType::Ptr mime = KMimeTypeFactory::self()->
                          findMimeTypeByName( KMimeType::defaultMimeType() );

    if (mime)
    {
        s_pDefaultMimeType = mime;
    }
    else
    {
        QString defaultMimeType = KMimeType::defaultMimeType();
        errorMissingMimeTypes( QStringList(defaultMimeType) );
        QString sDefaultMimeType = KGlobal::dirs()->resourceDirs("xdgdata-mime").first()+defaultMimeType+".xml";
        s_pDefaultMimeType = new KMimeType( sDefaultMimeType, defaultMimeType, "mime" );
    }
}

KMimeType::Ptr KMimeType::defaultMimeTypePtr()
{
    if ( !s_pDefaultMimeType ) // we need a default type first
        buildDefaultType();
    return s_pDefaultMimeType;
}

bool KMimeType::isDefault() const
{
    return this == defaultMimeTypePtr().data();
}

/**
 * This function makes sure that vital mime types are installed.
 */
void KMimeType::checkEssentialMimeTypes()
{
    static bool s_bChecked = false;

    if ( s_bChecked ) // already done
        return;
    if ( !s_pDefaultMimeType ) // we need a default type first
        KMimeType::buildDefaultType();

  s_bChecked = true; // must be done before building mimetypes

  // No Mime-Types installed ?
  // Lets do some rescue here.
  if ( !KMimeTypeFactory::self()->checkMimeTypes() )
  {
    // Note that this messagebox is queued, so it will only be shown once getting back to the event loop

    // No mimetypes installed? Are you setting XDG_DATA_DIRS without including /usr/share in it?
    KMessage::message( KMessage::Error, i18n( "No mime types installed. Check that shared-mime-info is installed, and that XDG_DATA_DIRS is not set, or includes /usr/share." ) );
    return; // no point in going any further
  }

  QStringList missingMimeTypes;

#ifndef Q_OS_WIN
  if ( !KMimeType::mimeType( "inode/directory" ) )
    missingMimeTypes.append( "inode/directory" );
  //if ( !KMimeType::mimeType( "inode/directory-locked" ) )
  //  missingMimeTypes.append( "inode/directory-locked" );
  if ( !KMimeType::mimeType( "inode/blockdevice" ) )
    missingMimeTypes.append( "inode/blockdevice" );
  if ( !KMimeType::mimeType( "inode/chardevice" ) )
    missingMimeTypes.append( "inode/chardevice" );
  if ( !KMimeType::mimeType( "inode/socket" ) )
    missingMimeTypes.append( "inode/socket" );
  if ( !KMimeType::mimeType( "inode/fifo" ) )
    missingMimeTypes.append( "inode/fifo" );
#endif
  if ( !KMimeType::mimeType( "application/x-shellscript" ) )
    missingMimeTypes.append( "application/x-shellscript" );
  if ( !KMimeType::mimeType( "application/x-executable" ) )
    missingMimeTypes.append( "application/x-executable" );
  if ( !KMimeType::mimeType( "application/x-desktop" ) )
    missingMimeTypes.append( "application/x-desktop" );

  if (!missingMimeTypes.isEmpty())
    errorMissingMimeTypes(missingMimeTypes);
}

KMimeType::Ptr KMimeType::mimeType( const QString& _name, FindByNameOption options )
{
    return KMimeTypeFactory::self()->findMimeTypeByName( _name, options );
}

KMimeType::List KMimeType::allMimeTypes()
{
    return KMimeTypeFactory::self()->allMimeTypes();
}

bool KMimeType::isBufferBinaryData(const QByteArray& data)
{
    // Check the first 32 bytes (see shared-mime spec)
    const char* p = data.data();
    const int end = qMin(32, data.size());
    for (int i = 0; i < end; ++i) {
        if (p[i] < 32 && p[i] != 9 && p[i] != 10 && p[i] != 13) // ASCII control character
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
        if ( KDE_stat( QFile::encodeName(path), &buff ) != -1 )
            mode = buff.st_mode;
    }

    if ( S_ISDIR( mode ) ) {
        // KDE4 TODO: use an overlay instead
#if 0
        // Special hack for local files. We want to see whether we
        // are allowed to enter the directory
        if ( is_local_file )
        {
            if ( access( QFile::encodeName(path), R_OK ) == -1 )
                return KMimeType::mimeType( "inode/directory-locked" );
        }
#endif
        return KMimeType::mimeType( "inode/directory" );
    }
    if ( S_ISCHR( mode ) )
        return KMimeType::mimeType( "inode/chardevice" );
    if ( S_ISBLK( mode ) )
        return KMimeType::mimeType( "inode/blockdevice" );
    if ( S_ISFIFO( mode ) )
        return KMimeType::mimeType( "inode/fifo" );
    if ( S_ISSOCK( mode ) )
        return KMimeType::mimeType( "inode/socket" );
#ifdef Q_OS_WIN
    // FIXME: distinguish between mounted & unmounted
    int size = path.size();
    if ( size == 2 || size == 3 ) {
        unsigned int type = GetDriveTypeW( (LPCWSTR) path.utf16() );
        switch( type ) {
            case DRIVE_REMOVABLE:
                return KMimeType::mimeType( "media/floppy_mounted" );
            case DRIVE_FIXED:
                return KMimeType::mimeType( "media/hdd_mounted" );
            case DRIVE_REMOTE:
                return KMimeType::mimeType( "media/smb_mounted" );
            case DRIVE_CDROM:
                return KMimeType::mimeType( "media/cdrom_mounted" );
            case DRIVE_RAMDISK:
                return KMimeType::mimeType( "media/hdd_mounted" );
            default:
                break;
        };
    }
#endif
    // remote executable file? stop here (otherwise findFromContent can do that better for local files)
    if ( !is_local_file && S_ISREG( mode ) && ( mode & ( S_IXUSR | S_IXGRP | S_IXOTH ) ) )
        return KMimeType::mimeType( "application/x-executable" );

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
    const QString path = _url.path();

    if (accuracy)
        *accuracy = 100;

    // Look at mode first
    KMimeType::Ptr mimeFromMode = findFromMode( path, mode, is_local_file );
    if (mimeFromMode)
        return mimeFromMode;

    // First try to find out by looking at the filename (if there's one)
    const QString fileName( _url.fileName() );
    QList<KMimeType::Ptr> mimeList;
    if ( !fileName.isEmpty() && !path.endsWith( '/' ) ) {
        // and if we can trust it (e.g. don't trust *.pl over HTTP, could be anything)
        if ( is_local_file || _url.hasSubUrl() || // Explicitly trust suburls
             KProtocolInfo::determineMimetypeFromExtension( _url.protocol() ) ) {
            mimeList = KMimeTypeFactory::self()->findFromFileName( fileName );
            // Found one glob match exactly: OK, use that.
            // We disambiguate multiple glob matches by sniffing, below.
            if ( mimeList.count() == 1 ) {
                return mimeList.first();
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
        KMimeType::Ptr mime = KMimeTypeFactory::self()->findFromContent(
            device, KMimeTypeFactory::AllRules, &magicAccuracy, beginning );
        // mime can't be 0, except in case of install problems.
        // However we get magicAccuracy==0 for octet-stream, i.e. no magic match found.
        //kDebug(7009) << "findFromContent said" << (mime?mime->name():QString()) << "with accuracy" << magicAccuracy;
        if (mime && magicAccuracy > 0) {

            // Disambiguate conflicting extensions (if magic found something and the magicrule was <80)
            if (magicAccuracy < 80 && !mimeList.isEmpty()) {
                // "for glob_match in glob_matches:"
                // "if glob_match is subclass or equal to sniffed_type, use glob_match"
                const QString sniffedMime = mime->name();
                foreach(const KMimeType::Ptr &mimeFromPattern, mimeList) {
                    //kDebug(7009) << "sniffedMime=" << sniffedMime << "mimeFromPattern=" << mimeFromPattern->name();
                    if (mimeFromPattern->is(sniffedMime)) {
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
        return mimeList.first();
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
    if ( path.endsWith( '/' ) || path.isEmpty() ) {
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
        QFile file(url.path());
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

QString KMimeType::extractKnownExtension(const QString &fileName)
{
    QString pattern;
    KMimeTypeFactory::self()->findFromFileName( fileName, &pattern );
    return pattern;
}

KMimeType::Ptr KMimeType::findByContent( const QByteArray &data, int *accuracy )
{
    QBuffer buffer(const_cast<QByteArray *>(&data));
    buffer.open(QIODevice::ReadOnly);
    QByteArray cache;
    return KMimeTypeFactory::self()->findFromContent(
        &buffer, KMimeTypeFactory::AllRules, accuracy, cache );
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
    return KMimeTypeFactory::self()->findFromContent(
        &device, KMimeTypeFactory::AllRules, accuracy, cache );
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
    // Warning adding/removing fields here involves a binary incompatible change - update version
    // number in ksycoca.h
    _str << m_lstPatterns << QString() << m_parentMimeTypes;
}

QVariant KMimeTypePrivate::property( const QString& _name ) const
{
    if ( _name == "Patterns" )
        return QVariant( m_lstPatterns );
    if ( _name == "Icon" )
        return QVariant( iconName(KUrl()) );

    return KServiceTypePrivate::property( _name );
}

QStringList KMimeTypePrivate::propertyNames() const
{
    QStringList res = KServiceTypePrivate::propertyNames();
    res.append( "Patterns" );
    res.append( "Icon" );
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
    if ( i == unknown || i.isEmpty() || mt == defaultMimeTypePtr()
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
    // this method will be called quite often, so better not read the config
    // again and again.
    static bool useFavIcons = true;
    static bool check = true;
    if ( check ) {
        check = false;
        KConfigGroup cg( KGlobal::config(), "HTML Settings" );
        useFavIcons = cg.readEntry("EnableFavicon", true);
    }

    if ( url.isLocalFile() || !url.protocol().startsWith("http")
         || !useFavIcons )
        return QString();

    QDBusInterface kded( "org.kde.kded", "/modules/favicons", "org.kde.FavIcon" );
    QDBusReply<QString> result = kded.call( "iconForUrl", url.url() );
    return result;              // default is QString()
}

QString KMimeType::comment( const KUrl &url) const
{
    Q_D(const KMimeType);
    return d->comment(url);
}

QString KMimeType::parentMimeType() const
{
    Q_D(const KMimeType);

    if (!d->m_parentMimeTypes.isEmpty())
        return d->m_parentMimeTypes.first();
    return d->fallbackParent();
}

QString KMimeTypePrivate::fallbackParent() const
{
    const QString myGroup = m_strName.left(m_strName.indexOf('/'));
    // All text/* types are subclasses of text/plain.
    if (myGroup == "text" && m_strName != "text/plain")
        return "text/plain";
    // All real-file mimetypes implicitly derive from application/octet-stream
    if (myGroup != "inode" &&
        // kde extensions
        myGroup != "all" && myGroup != "fonts" && myGroup != "print" && myGroup != "uri"
        && m_strName != "application/octet-stream") {
        return "application/octet-stream";
    }
    return QString();
}

bool KMimeTypePrivate::inherits(const QString& mime) const
{
    if (m_strName == mime) {
        return true;
    }
    foreach( const QString& parent, parentMimeTypes() ) {
        KMimeType::Ptr parentMime = KMimeTypeFactory::self()->findMimeTypeByName(parent);
        if (!parentMime) // error
            return false;
        if (parentMime->d_func()->inherits(mime)) // recurse
            return true;
    }
    return false;
}

bool KMimeType::is( const QString& mimeTypeName ) const
{
    Q_D(const KMimeType);
    if (name() == mimeTypeName)
        return true;
    QString mime = KMimeTypeFactory::self()->resolveAlias(mimeTypeName);
    if (mime.isEmpty())
        mime = mimeTypeName;
    return d->inherits(mime);
}

QStringList KMimeType::parentMimeTypes() const
{
    Q_D(const KMimeType);
    return d->parentMimeTypes();
}

QStringList KMimeTypePrivate::parentMimeTypes() const
{
    QStringList parents = m_parentMimeTypes;
    if (parents.isEmpty()) {
        const QString myParent = fallbackParent();
        if (!myParent.isEmpty())
            parents.append(myParent);
    }
    return parents;
}

void KMimeTypePrivate::collectParentMimeTypes(QStringList& allParents) const
{
    QStringList parents = parentMimeTypes();
    Q_FOREACH(const QString& parent, parents) {
        // I would use QSet, but since order matters I better not
        if (!allParents.contains(parent))
            allParents.append(parent);
    }
    // We want a breadth-first search, so that the least-specific parent (octet-stream) is last
    // This means iterating twice, unfortunately.
    Q_FOREACH(const QString& parent, parents) {
        KMimeType::Ptr parentMime = KMimeTypeFactory::self()->findMimeTypeByName(parent);
        if (parentMime)
            parentMime->d_func()->collectParentMimeTypes(allParents);
    }
}

QStringList KMimeType::allParentMimeTypes() const
{
    QStringList allParents;
    const QString canonical = KMimeTypeFactory::self()->resolveAlias(name());
    if (!canonical.isEmpty())
        allParents.append(canonical);
    d_func()->collectParentMimeTypes(allParents);
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
    return d->m_lstPatterns;
}

void KMimeType::setPatterns(const QStringList& patterns)
{
    Q_D(KMimeType);
    d->m_lstPatterns = patterns;
}

void KMimeType::setParentMimeType(const QString& parent)
{
    Q_D(KMimeType);
    // kbuildmimetypefactory calls this multiple times, for each parent mimetype
    d->m_parentMimeTypes.append(parent);
}

void KMimeType::internalClearData()
{
    Q_D(KMimeType);
    // Clear the data that KBuildMimeTypeFactory is going to refill - and only that data.
    d->m_parentMimeTypes.clear();
    d->m_lstPatterns.clear();
}
