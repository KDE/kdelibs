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
#include "qmimedatabase.h"

#include <kdebug.h>
#include <kde_file.h> // KDE::stat
#include <kdeversion.h> // KDE_MAKE_VERSION
#include <klocalizedstring.h>
#include <qstandardpaths.h>
#include <kurl.h>

#include <QtCore/QFile>
#include <QtDBus/QtDBus>
#include <QBuffer>

#ifndef S_ISSOCK
#define S_ISSOCK(x) false
#endif

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

KMimeType::Ptr KMimeType::mimeType(const QString& name, FindByNameOption options)
{
    Q_UNUSED(options);
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForName(name);
    if (mime.isValid()) {
        return KMimeType::Ptr(new KMimeType(mime));
    } else {
        return KMimeType::Ptr();
    }
}

KMimeType::List KMimeType::allMimeTypes()
{
    // This could be done faster...
    KMimeType::List lst;
    QMimeDatabase db;
    Q_FOREACH(const QMimeType& mimeType, db.allMimeTypes()) {
        Q_ASSERT(!mimeType.name().startsWith(QLatin1String("x-scheme-handler")));
        lst.append(KMimeType::Ptr(new KMimeType(mimeType)));
    }
    return lst;
}

// TODO used outside of kmimetype?
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

// the windows-specific code, and the locked directory, are missing from QMimeDatabase.
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
 Note: in KDE we want the file views to sniff in a delayed manner.
 So there's also a fast mode which is:
  if no glob matches, or if more than one glob matches, use default mimetype and mark as "can be refined".
*/

KMimeType::Ptr KMimeType::findByUrl( const QUrl& url, mode_t mode,
                                     bool is_local_file, bool fast_mode,
                                     int *accuracy )
{
    Q_UNUSED(mode); // special devices only matter locally; caller can use S_ISDIR by itself.
    Q_UNUSED(is_local_file); // QUrl can tell us...
    QMimeDatabase db;
    if (accuracy)
        *accuracy = 80; // not supported anymore; was it really used for anything?
    if (fast_mode) {
        return KMimeType::Ptr(new KMimeType(db.mimeTypeForFile(url.path(), QMimeDatabase::MatchExtension)));
    }
    return KMimeType::Ptr(new KMimeType(db.mimeTypeForUrl(url)));
}

KMimeType::Ptr KMimeType::findByPath( const QString& path, mode_t mode,
                                      bool fast_mode, int* accuracy )
{
    Q_UNUSED(mode); // special devices only matter locally; caller can use S_ISDIR by itself.
    QMimeDatabase db;
    if (accuracy)
        *accuracy = 80; // not supported anymore; was it really used for anything?
    if (fast_mode) {
        return KMimeType::Ptr(new KMimeType(db.mimeTypeForFile(path, QMimeDatabase::MatchExtension)));
    }
    return KMimeType::Ptr(new KMimeType(db.mimeTypeForFile(path)));
}

KMimeType::Ptr KMimeType::findByNameAndContent( const QString& name, const QByteArray& data,
                                                mode_t mode, int* accuracy )
{
    Q_UNUSED(mode); // If we have data, then this is a regular file anyway...
    if (accuracy)
        *accuracy = 80; // not supported anymore; was it really used for anything?
    QMimeDatabase db;
    return KMimeType::Ptr(new KMimeType(db.mimeTypeForFileNameAndData(name, data)));
}

KMimeType::Ptr KMimeType::findByNameAndContent( const QString& name, QIODevice* device,
                                                mode_t mode, int* accuracy )
{
    Q_UNUSED(mode); // If we have data, then this is a regular file anyway...
    if (accuracy)
        *accuracy = 80; // not supported anymore; was it really used for anything?
    QMimeDatabase db;
    return KMimeType::Ptr(new KMimeType(db.mimeTypeForFileNameAndData(name, device)));
}

QString KMimeType::extractKnownExtension(const QString &fileName)
{
    QMimeDatabase db;
    return db.suffixForFileName(fileName);
}

KMimeType::Ptr KMimeType::findByContent( const QByteArray &data, int *accuracy )
{
    QMimeDatabase db;
    if (accuracy)
        *accuracy = 80; // not supported anymore; was it really used for anything?
    return KMimeType::Ptr(new KMimeType(db.mimeTypeForData(data)));
}

KMimeType::Ptr KMimeType::findByContent( QIODevice* device, int* accuracy )
{
    QMimeDatabase db;
    if (accuracy)
        *accuracy = 80; // not supported anymore; was it really used for anything?
    return KMimeType::Ptr(new KMimeType(db.mimeTypeForData(device)));
}

KMimeType::Ptr KMimeType::findByFileContent( const QString &fileName, int *accuracy )
{
    QFile device(fileName);
#if 1
    // Look at mode first
    KMimeType::Ptr mimeFromMode = findFromMode( fileName, 0, true );
    if (mimeFromMode) {
        if (accuracy)
            *accuracy = 100;
        return mimeFromMode;
    }
#endif
    QMimeDatabase db;
    KMimeType::Ptr mime(new KMimeType(db.mimeTypeForData(&device)));
    if (accuracy)
        *accuracy = mime->isDefault() ? 0 : 80; // not supported anymore; was it really used for anything?
    return mime;
}

bool KMimeType::isBinaryData( const QString &fileName )
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return false; // err, whatever
    const QByteArray data = file.read(32);
    return isBufferBinaryData(data);
}

KMimeType::KMimeType(const QMimeType& mime)
    : d_ptr(new KMimeTypePrivate(mime))
{
}

KMimeType::~KMimeType()
{
    delete d_ptr;
}

QString KMimeType::favIconForUrl(const QUrl& url)
{
    if (url.isLocalFile()
        || !url.scheme().startsWith(QLatin1String("http"))
        || !KMimeTypeRepository::self()->useFavIcons())
        return QString();

    QDBusInterface kded( QString::fromLatin1("org.kde.kded5"),
                         QString::fromLatin1("/modules/favicons"),
                         QString::fromLatin1("org.kde.FavIcon") );
    QDBusReply<QString> result = kded.call(QString::fromLatin1("iconForUrl"), url.toString());
    return result;              // default is QString()
}

QString KMimeType::comment() const
{
    return d_ptr->m_qmime.comment();
}

#ifndef KDE_NO_DEPRECATED
QString KMimeType::parentMimeType() const
{
    const QStringList parents = d_ptr->m_qmime.parentMimeTypes();
    if (!parents.isEmpty())
        return parents.first();
    return QString();
}
#endif

bool KMimeType::is(const QString& mimeTypeName) const
{
    return d_ptr->m_qmime.inherits(mimeTypeName);
}

QStringList KMimeType::parentMimeTypes() const
{
    return d_ptr->m_qmime.parentMimeTypes();
}

QStringList KMimeType::allParentMimeTypes() const
{
    return d_ptr->m_qmime.allAncestors();
}

QString KMimeType::defaultMimeType()
{
    return QLatin1String( "application/octet-stream" );
}

QString KMimeType::iconName() const
{
    return d_ptr->m_qmime.iconName();
}

QStringList KMimeType::patterns() const
{
    return d_ptr->m_qmime.globPatterns();
}

// TODO MOVE TO keditfiletype/mimetypedata.cpp
QString KMimeType::userSpecifiedIconName() const
{
    //d->ensureXmlDataLoaded();
    //return d->m_iconName;
    return QString();
}

int KMimeType::sharedMimeInfoVersion()
{
    return KMimeTypeRepository::self()->sharedMimeInfoVersion();
}

QString KMimeType::mainExtension() const
{
#if 1 // HACK START - can be removed once shared-mime-info >= 0.70 is used/required.
    // The idea was: first usable pattern from m_lstPatterns.
    // But update-mime-database makes a mess of the order of the patterns,
    // because it uses a hash internally.
    static const struct { const char* mime; const char* extension; } s_hardcodedMimes[] = {
        { "text/plain", ".txt" } };
    if (patterns().count() > 1) {
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

/*
int KMimeTypePrivate::serviceOffersOffset() const
{
    return KMimeTypeFactory::self()->serviceOffersOffset(name());
}
*/

QString KMimeType::name() const
{
    return d_ptr->m_qmime.name();
}
