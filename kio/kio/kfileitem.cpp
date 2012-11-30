/* This file is part of the KDE project
   Copyright (C) 1999-2011 David Faure <faure@kde.org>
   2001 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include "kfileitem.h"

#include <config-kio.h>

#include <sys/time.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>
#include <unistd.h>

#include <QtCore/QDate>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QFile>
#include <QtCore/QMap>
#include <QApplication>
#include <QPalette>
#include <QTextDocument>
#include <QMimeDatabase>
#include <qurlpathinfo.h>

#include <kdebug.h>
#include <kfilemetainfo.h>
#include <kiconloader.h>
#include <klocale.h>
#include <krun.h>
#include <kde_file.h>
#include <kdesktopfile.h>
#include <kmountpoint.h>
#include <kconfiggroup.h>
#ifndef Q_OS_WIN
#include <knfsshare.h>
#include <ksambashare.h>
#endif
#include <kfilesystemtype_p.h>

class KFileItemPrivate : public QSharedData
{
public:
    KFileItemPrivate(const KIO::UDSEntry& entry,
                     mode_t mode, mode_t permissions,
                     const QUrl& itemOrDirUrl,
                     bool urlIsDirectory,
                     bool delayedMimeTypes)
        : m_entry( entry ),
          m_url(itemOrDirUrl),
          m_strName(),
          m_strText(),
          m_iconName(),
          m_strLowerCaseName(),
          m_mimeType(),
          m_fileMode( mode ),
          m_permissions( permissions ),
          m_bMarked( false ),
          m_bLink( false ),
          m_bIsLocalUrl(itemOrDirUrl.isLocalFile()),
          m_bMimeTypeKnown( false ),
          m_delayedMimeTypes( delayedMimeTypes ),
          m_useIconNameCache(false),
          m_hidden(Auto),
          m_slow(SlowUnknown)
    {
        if (entry.count() != 0) {
            readUDSEntry( urlIsDirectory );
        } else {
            Q_ASSERT(!urlIsDirectory);
            m_strName = QUrlPathInfo(itemOrDirUrl).fileName();
            m_strText = KIO::decodeFileName( m_strName );
        }
        init();
    }

    ~KFileItemPrivate()
    {
    }

    /**
     * Computes the text and mode from the UDSEntry
     * Called by constructor, but can be called again later
     * Nothing does that anymore though (I guess some old KonqFileItem did)
     * so it's not a protected method of the public class anymore.
     */
    void init();

    QString localPath() const;
    KIO::filesize_t size() const;
    KDateTime time( KFileItem::FileTimes which ) const;
    void setTime(KFileItem::FileTimes which, long long time_t_val) const;
    bool cmp( const KFileItemPrivate & item ) const;
    QString user() const;
    QString group() const;
    bool isSlow() const;

    /**
     * Extracts the data from the UDSEntry member and updates the KFileItem
     * accordingly.
     */
    void readUDSEntry( bool _urlIsDirectory );

    /**
     * Parses the given permission set and provides it for access()
     */
    QString parsePermissions( mode_t perm ) const;

    /**
     * The UDSEntry that contains the data for this fileitem, if it came from a directory listing.
     */
    mutable KIO::UDSEntry m_entry;
    /**
     * The url of the file
     */
    QUrl m_url;

    /**
     * The text for this item, i.e. the file name without path,
     */
    QString m_strName;

    /**
     * The text for this item, i.e. the file name without path, decoded
     * ('%%' becomes '%', '%2F' becomes '/')
     */
    QString m_strText;

    /**
     * The icon name for this item.
     */
    mutable QString m_iconName;

    /**
     * The filename in lower case (to speed up sorting)
     */
    mutable QString m_strLowerCaseName;

    /**
     * The mimetype of the file
     */
    mutable QMimeType m_mimeType;

    /**
     * The file mode
     */
    mode_t m_fileMode;
    /**
     * The permissions
     */
    mode_t m_permissions;

    /**
     * Marked : see mark()
     */
    bool m_bMarked:1;
    /**
     * Whether the file is a link
     */
    bool m_bLink:1;
    /**
     * True if local file
     */
    bool m_bIsLocalUrl:1;

    mutable bool m_bMimeTypeKnown:1;
    bool m_delayedMimeTypes:1;

    /** True if m_iconName should be used as cache. */
    mutable bool m_useIconNameCache:1;

    // Auto: check leading dot.
    enum { Auto, Hidden, Shown } m_hidden:3;

    // Slow? (nfs/smb/ssh)
    mutable enum { SlowUnknown, Fast, Slow } m_slow:3;

    // For special case like link to dirs over FTP
    QString m_guessedMimeType;
    mutable QString m_access;
#ifndef KDE_NO_DEPRECATED
    QMap<const void*, void*> m_extra; // DEPRECATED
#endif
    mutable KFileMetaInfo m_metaInfo;

    enum { NumFlags = KFileItem::CreationTime + 1 };
    mutable KDateTime m_time[3];
};

void KFileItemPrivate::init()
{
    m_access.clear();
    //  metaInfo = KFileMetaInfo();

    // determine mode and/or permissions if unknown
    // TODO: delay this until requested
    if ( m_fileMode == KFileItem::Unknown || m_permissions == KFileItem::Unknown )
    {
        mode_t mode = 0;
        if ( m_url.isLocalFile() )
        {
            /* directories may not have a slash at the end if
             * we want to stat() them; it requires that we
             * change into it .. which may not be allowed
             * stat("/is/unaccessible")  -> rwx------
             * stat("/is/unaccessible/") -> EPERM            H.Z.
             * This is the reason for the -1
             */
            KDE_struct_stat buf;
            const QString path = QUrlPathInfo(m_url).localPath(QUrlPathInfo::StripTrailingSlash);
            if ( KDE::lstat( path, &buf ) == 0 )
            {
                mode = buf.st_mode;
                if ( S_ISLNK( mode ) )
                {
                    m_bLink = true;
                    if ( KDE::stat( path, &buf ) == 0 )
                        mode = buf.st_mode;
                    else // link pointing to nowhere (see kio/file/file.cc)
                        mode = (S_IFMT-1) | S_IRWXU | S_IRWXG | S_IRWXO;
                }
                // While we're at it, store the times
                setTime(KFileItem::ModificationTime, buf.st_mtime);
                setTime(KFileItem::AccessTime, buf.st_atime);
                if ( m_fileMode == KFileItem::Unknown )
                    m_fileMode = mode & S_IFMT; // extract file type
                if ( m_permissions == KFileItem::Unknown )
                    m_permissions = mode & 07777; // extract permissions
            } else {
                kDebug() << path << "does not exist anymore";
            }
        }
    }
}

void KFileItemPrivate::readUDSEntry( bool _urlIsDirectory )
{
    // extract fields from the KIO::UDS Entry

    m_fileMode = m_entry.numberValue( KIO::UDSEntry::UDS_FILE_TYPE );
    m_permissions = m_entry.numberValue( KIO::UDSEntry::UDS_ACCESS );
    m_strName = m_entry.stringValue( KIO::UDSEntry::UDS_NAME );

    const QString displayName = m_entry.stringValue( KIO::UDSEntry::UDS_DISPLAY_NAME );
    if (!displayName.isEmpty())
      m_strText = displayName;
    else
      m_strText = KIO::decodeFileName( m_strName );

    const QString urlStr = m_entry.stringValue( KIO::UDSEntry::UDS_URL );
    const bool UDS_URL_seen = !urlStr.isEmpty();
    if ( UDS_URL_seen ) {
        m_url = QUrl(urlStr);
        if ( m_url.isLocalFile() )
            m_bIsLocalUrl = true;
    }
    QMimeDatabase db;
    const QString mimeTypeStr = m_entry.stringValue( KIO::UDSEntry::UDS_MIME_TYPE );
    m_bMimeTypeKnown = !mimeTypeStr.isEmpty();
    if ( m_bMimeTypeKnown )
        m_mimeType = db.mimeTypeForName(mimeTypeStr);

    m_guessedMimeType = m_entry.stringValue( KIO::UDSEntry::UDS_GUESSED_MIME_TYPE );
    m_bLink = !m_entry.stringValue( KIO::UDSEntry::UDS_LINK_DEST ).isEmpty(); // we don't store the link dest

    const int hiddenVal = m_entry.numberValue( KIO::UDSEntry::UDS_HIDDEN, -1 );
    m_hidden = hiddenVal == 1 ? Hidden : ( hiddenVal == 0 ? Shown : Auto );

    if (_urlIsDirectory && !UDS_URL_seen && !m_strName.isEmpty() && m_strName != QLatin1String(".")) {
        m_url = QUrlPathInfo::addPathToUrl(m_url, m_strName);
    }

    m_iconName.clear();
}

inline //because it is used only in one place
KIO::filesize_t KFileItemPrivate::size() const
{
    // Extract it from the KIO::UDSEntry
    long long fieldVal = m_entry.numberValue( KIO::UDSEntry::UDS_SIZE, -1 );
    if ( fieldVal != -1 ) {
        return fieldVal;
    }

    // If not in the KIO::UDSEntry, or if UDSEntry empty, use stat() [if local URL]
    if ( m_bIsLocalUrl ) {
        KDE_struct_stat buf;
        if (KDE::stat(QUrlPathInfo(m_url).localPath(QUrlPathInfo::StripTrailingSlash), &buf) == 0)
            return buf.st_size;
    }
    return 0;
}

void KFileItemPrivate::setTime(KFileItem::FileTimes mappedWhich, long long time_t_val) const
{
    m_time[mappedWhich].setTime_t(time_t_val);
    m_time[mappedWhich] = m_time[mappedWhich].toLocalZone(); // #160979
}

KDateTime KFileItemPrivate::time( KFileItem::FileTimes mappedWhich ) const
{
    if ( !m_time[mappedWhich].isNull() )
        return m_time[mappedWhich];

    // Extract it from the KIO::UDSEntry
    long long fieldVal = -1;
    switch ( mappedWhich ) {
    case KFileItem::ModificationTime:
        fieldVal = m_entry.numberValue( KIO::UDSEntry::UDS_MODIFICATION_TIME, -1 );
        break;
    case KFileItem::AccessTime:
        fieldVal = m_entry.numberValue( KIO::UDSEntry::UDS_ACCESS_TIME, -1 );
        break;
    case KFileItem::CreationTime:
        fieldVal = m_entry.numberValue( KIO::UDSEntry::UDS_CREATION_TIME, -1 );
        break;
    }
    if ( fieldVal != -1 ) {
        setTime(mappedWhich, fieldVal);
        return m_time[mappedWhich];
    }

    // If not in the KIO::UDSEntry, or if UDSEntry empty, use stat() [if local URL]
    if ( m_bIsLocalUrl )
    {
        KDE_struct_stat buf;
        if (KDE::stat(QUrlPathInfo(m_url).localPath(QUrlPathInfo::StripTrailingSlash), &buf) == 0) {
            setTime(KFileItem::ModificationTime, buf.st_mtime);
            setTime(KFileItem::AccessTime, buf.st_atime);
            m_time[KFileItem::CreationTime] = KDateTime();
            return m_time[mappedWhich];
        }
    }
    return KDateTime();
}

inline //because it is used only in one place
bool KFileItemPrivate::cmp( const KFileItemPrivate & item ) const
{
#if 0
    kDebug() << "Comparing" << m_url << "and" << item.m_url;
    kDebug() << " name" << (m_strName == item.m_strName);
    kDebug() << " local" << (m_bIsLocalUrl == item.m_bIsLocalUrl);
    kDebug() << " mode" << (m_fileMode == item.m_fileMode);
    kDebug() << " perm" << (m_permissions == item.m_permissions);
    kDebug() << " UDS_USER" << (user() == item.user());
    kDebug() << " UDS_GROUP" << (group() == item.group());
    kDebug() << " UDS_EXTENDED_ACL" << (m_entry.stringValue( KIO::UDSEntry::UDS_EXTENDED_ACL ) == item.m_entry.stringValue( KIO::UDSEntry::UDS_EXTENDED_ACL ));
    kDebug() << " UDS_ACL_STRING" << (m_entry.stringValue( KIO::UDSEntry::UDS_ACL_STRING ) == item.m_entry.stringValue( KIO::UDSEntry::UDS_ACL_STRING ));
    kDebug() << " UDS_DEFAULT_ACL_STRING" << (m_entry.stringValue( KIO::UDSEntry::UDS_DEFAULT_ACL_STRING ) == item.m_entry.stringValue( KIO::UDSEntry::UDS_DEFAULT_ACL_STRING ));
    kDebug() << " m_bLink" << (m_bLink == item.m_bLink);
    kDebug() << " m_hidden" << (m_hidden == item.m_hidden);
    kDebug() << " size" << (size() == item.size());
    kDebug() << " ModificationTime" << (time(KFileItem::ModificationTime) == item.time(KFileItem::ModificationTime));
    kDebug() << " UDS_ICON_NAME" << (m_entry.stringValue( KIO::UDSEntry::UDS_ICON_NAME ) == item.m_entry.stringValue( KIO::UDSEntry::UDS_ICON_NAME ));
#endif
    return ( m_strName == item.m_strName
             && m_bIsLocalUrl == item.m_bIsLocalUrl
             && m_fileMode == item.m_fileMode
             && m_permissions == item.m_permissions
             && user() == item.user()
             && group() == item.group()
             && m_entry.stringValue( KIO::UDSEntry::UDS_EXTENDED_ACL ) == item.m_entry.stringValue( KIO::UDSEntry::UDS_EXTENDED_ACL )
             && m_entry.stringValue( KIO::UDSEntry::UDS_ACL_STRING ) == item.m_entry.stringValue( KIO::UDSEntry::UDS_ACL_STRING )
             && m_entry.stringValue( KIO::UDSEntry::UDS_DEFAULT_ACL_STRING ) == item.m_entry.stringValue( KIO::UDSEntry::UDS_DEFAULT_ACL_STRING )
             && m_bLink == item.m_bLink
             && m_hidden == item.m_hidden
             && size() == item.size()
             && time(KFileItem::ModificationTime) == item.time(KFileItem::ModificationTime) // TODO only if already known!
             && m_entry.stringValue( KIO::UDSEntry::UDS_ICON_NAME ) == item.m_entry.stringValue( KIO::UDSEntry::UDS_ICON_NAME )
        );

    // Don't compare the mimetypes here. They might not be known, and we don't want to
    // do the slow operation of determining them here.
}

inline //because it is used only in one place
QString KFileItemPrivate::parsePermissions(mode_t perm) const
{
    static char buffer[ 12 ];

    char uxbit,gxbit,oxbit;

    if ( (perm & (S_IXUSR|S_ISUID)) == (S_IXUSR|S_ISUID) )
        uxbit = 's';
    else if ( (perm & (S_IXUSR|S_ISUID)) == S_ISUID )
        uxbit = 'S';
    else if ( (perm & (S_IXUSR|S_ISUID)) == S_IXUSR )
        uxbit = 'x';
    else
        uxbit = '-';

    if ( (perm & (S_IXGRP|S_ISGID)) == (S_IXGRP|S_ISGID) )
        gxbit = 's';
    else if ( (perm & (S_IXGRP|S_ISGID)) == S_ISGID )
        gxbit = 'S';
    else if ( (perm & (S_IXGRP|S_ISGID)) == S_IXGRP )
        gxbit = 'x';
    else
        gxbit = '-';

    if ( (perm & (S_IXOTH|S_ISVTX)) == (S_IXOTH|S_ISVTX) )
        oxbit = 't';
    else if ( (perm & (S_IXOTH|S_ISVTX)) == S_ISVTX )
        oxbit = 'T';
    else if ( (perm & (S_IXOTH|S_ISVTX)) == S_IXOTH )
        oxbit = 'x';
    else
        oxbit = '-';

    // Include the type in the first char like kde3 did; people are more used to seeing it,
    // even though it's not really part of the permissions per se.
    if (m_bLink)
        buffer[0] = 'l';
    else if (m_fileMode != KFileItem::Unknown) {
        if (S_ISDIR(m_fileMode))
            buffer[0] = 'd';
        else if (S_ISSOCK(m_fileMode))
            buffer[0] = 's';
        else if (S_ISCHR(m_fileMode))
            buffer[0] = 'c';
        else if (S_ISBLK(m_fileMode))
            buffer[0] = 'b';
        else if (S_ISFIFO(m_fileMode))
            buffer[0] = 'p';
        else
            buffer[0] = '-';
    } else {
        buffer[0] = '-';
    }

    buffer[1] = ((( perm & S_IRUSR ) == S_IRUSR ) ? 'r' : '-' );
    buffer[2] = ((( perm & S_IWUSR ) == S_IWUSR ) ? 'w' : '-' );
    buffer[3] = uxbit;
    buffer[4] = ((( perm & S_IRGRP ) == S_IRGRP ) ? 'r' : '-' );
    buffer[5] = ((( perm & S_IWGRP ) == S_IWGRP ) ? 'w' : '-' );
    buffer[6] = gxbit;
    buffer[7] = ((( perm & S_IROTH ) == S_IROTH ) ? 'r' : '-' );
    buffer[8] = ((( perm & S_IWOTH ) == S_IWOTH ) ? 'w' : '-' );
    buffer[9] = oxbit;
    // if (hasExtendedACL())
    if (m_entry.contains(KIO::UDSEntry::UDS_EXTENDED_ACL)) {
        buffer[10] = '+';
        buffer[11] = 0;
    } else {
        buffer[10] = 0;
    }

    return QString::fromLatin1(buffer);
}


///////

KFileItem::KFileItem()
    : d(0)
{
}

KFileItem::KFileItem( const KIO::UDSEntry& entry, const QUrl& itemOrDirUrl,
                      bool delayedMimeTypes, bool urlIsDirectory )
    : d(new KFileItemPrivate(entry, KFileItem::Unknown, KFileItem::Unknown,
                             itemOrDirUrl, urlIsDirectory, delayedMimeTypes))
{
}

KFileItem::KFileItem( mode_t mode, mode_t permissions, const QUrl& url, bool delayedMimeTypes )
    : d(new KFileItemPrivate(KIO::UDSEntry(), mode, permissions,
                             url, false, delayedMimeTypes))
{
}

KFileItem::KFileItem( const QUrl &url, const QString &mimeType, mode_t mode )
    : d(new KFileItemPrivate(KIO::UDSEntry(), mode, KFileItem::Unknown,
                             url, false, false))
{
    d->m_bMimeTypeKnown = !mimeType.isEmpty();
    if (d->m_bMimeTypeKnown) {
        QMimeDatabase db;
        d->m_mimeType = db.mimeTypeForName(mimeType);
    }
}


KFileItem::KFileItem(const KFileItem& other)
    : d(other.d)
{
}

KFileItem::~KFileItem()
{
}

void KFileItem::refresh()
{
    if (!d) {
        kWarning() << "null item";
        return;
    }

    d->m_fileMode = KFileItem::Unknown;
    d->m_permissions = KFileItem::Unknown;
    d->m_metaInfo = KFileMetaInfo();
    d->m_hidden = KFileItemPrivate::Auto;
    refreshMimeType();

    // Basically, we can't trust any information we got while listing.
    // Everything could have changed...
    // Clearing m_entry makes it possible to detect changes in the size of the file,
    // the time information, etc.
    d->m_entry.clear();
    d->init();
}

void KFileItem::refreshMimeType()
{
    if (!d)
        return;

    d->m_mimeType = QMimeType();
    d->m_bMimeTypeKnown = false;
    d->m_iconName.clear();
}

void KFileItem::setUrl( const QUrl &url )
{
    if (!d) {
        kWarning() << "null item";
        return;
    }

    d->m_url = url;
    setName(QUrlPathInfo(url).fileName());
}

void KFileItem::setName( const QString& name )
{
    if (!d) {
        kWarning() << "null item";
        return;
    }

    d->m_strName = name;
    d->m_strText = KIO::decodeFileName( d->m_strName );
    if (d->m_entry.contains(KIO::UDSEntry::UDS_NAME))
        d->m_entry.insert(KIO::UDSEntry::UDS_NAME, d->m_strName); // #195385

}

QString KFileItem::linkDest() const
{
    if (!d)
        return QString();

    // Extract it from the KIO::UDSEntry
    const QString linkStr = d->m_entry.stringValue( KIO::UDSEntry::UDS_LINK_DEST );
    if ( !linkStr.isEmpty() )
        return linkStr;

    // If not in the KIO::UDSEntry, or if UDSEntry empty, use readlink() [if local URL]
    if ( d->m_bIsLocalUrl )
    {
        char buf[1000];
        const int n = readlink(QFile::encodeName(QUrlPathInfo(d->m_url).localPath(QUrlPathInfo::StripTrailingSlash)), buf, sizeof(buf)-1);
        if ( n != -1 )
        {
            buf[ n ] = 0;
            return QFile::decodeName( buf );
        }
    }
    return QString();
}

QString KFileItemPrivate::localPath() const
{
  if (m_bIsLocalUrl) {
    return m_url.toLocalFile();
  }

  // Extract the local path from the KIO::UDSEntry
  return m_entry.stringValue( KIO::UDSEntry::UDS_LOCAL_PATH );
}

QString KFileItem::localPath() const
{
    if (!d)
        return QString();

    return d->localPath();
}

KIO::filesize_t KFileItem::size() const
{
    if (!d)
        return 0;

    return d->size();
}

bool KFileItem::hasExtendedACL() const
{
    if (!d)
        return false;

    // Check if the field exists; its value doesn't matter
    return d->m_entry.contains(KIO::UDSEntry::UDS_EXTENDED_ACL);
}

KACL KFileItem::ACL() const
{
    if (!d)
        return KACL();

    if ( hasExtendedACL() ) {
        // Extract it from the KIO::UDSEntry
        const QString fieldVal = d->m_entry.stringValue( KIO::UDSEntry::UDS_ACL_STRING );
        if ( !fieldVal.isEmpty() )
            return KACL( fieldVal );
    }
    // create one from the basic permissions
    return KACL( d->m_permissions );
}

KACL KFileItem::defaultACL() const
{
    if (!d)
        return KACL();

    // Extract it from the KIO::UDSEntry
    const QString fieldVal = d->m_entry.stringValue( KIO::UDSEntry::UDS_DEFAULT_ACL_STRING );
    if ( !fieldVal.isEmpty() )
        return KACL(fieldVal);
    else
        return KACL();
}

KDateTime KFileItem::time( FileTimes which ) const
{
    if (!d)
        return KDateTime();

    return d->time(which);
}

#ifndef KDE_NO_DEPRECATED
time_t KFileItem::time( unsigned int which ) const
{
    if (!d)
        return 0;

    switch (which) {
    case KIO::UDSEntry::UDS_ACCESS_TIME:
        return d->time(AccessTime).toTime_t();
    case KIO::UDSEntry::UDS_CREATION_TIME:
        return d->time(CreationTime).toTime_t();
    case KIO::UDSEntry::UDS_MODIFICATION_TIME:
    default:
        return d->time(ModificationTime).toTime_t();
    }
}
#endif

QString KFileItem::user() const
{
    if (!d)
        return QString();

    return d->user();
}

QString KFileItemPrivate::user() const
{
    QString userName = m_entry.stringValue(KIO::UDSEntry::UDS_USER);
    if (userName.isEmpty() && m_bIsLocalUrl) {
        QFileInfo a(QUrlPathInfo(m_url).localPath(QUrlPathInfo::StripTrailingSlash));
        userName = QFileInfo(a.canonicalFilePath()).owner(); // get user of the link, if it's a link
        m_entry.insert(KIO::UDSEntry::UDS_USER, userName);
    }
    return userName;
}

QString KFileItem::group() const
{
    if (!d)
        return QString();

    return d->group();
}

QString KFileItemPrivate::group() const
{
    QString groupName = m_entry.stringValue( KIO::UDSEntry::UDS_GROUP );
    if (groupName.isEmpty() && m_bIsLocalUrl ) {
        QFileInfo a(QUrlPathInfo(m_url).localPath(QUrlPathInfo::StripTrailingSlash));
        groupName = QFileInfo(a.canonicalFilePath()).group(); // get group of the link, if it's a link
        m_entry.insert( KIO::UDSEntry::UDS_GROUP, groupName );
    }
    return groupName;
}

bool KFileItemPrivate::isSlow() const
{
    if (m_slow == SlowUnknown) {
        const QString path = localPath();
        if (!path.isEmpty()) {
            const KFileSystemType::Type fsType = KFileSystemType::fileSystemType(path);
            m_slow = (fsType == KFileSystemType::Nfs || fsType == KFileSystemType::Smb) ? Slow : Fast;
        } else {
            m_slow = Slow;
        }
    }
    return m_slow == Slow;
}

bool KFileItem::isSlow() const
{
    if (!d)
        return false;

    return d->isSlow();
}

QString KFileItem::mimetype() const
{
    if (!d)
        return QString();

    KFileItem * that = const_cast<KFileItem *>(this);
    return that->determineMimeType().name();
}

QMimeType KFileItem::determineMimeType() const
{
    if (!d)
        return QMimeType();

    if (!d->m_mimeType.isValid() || !d->m_bMimeTypeKnown) {
        QMimeDatabase db;
        bool isLocalUrl;
        const QUrl url = mostLocalUrl(isLocalUrl);
        d->m_mimeType = db.mimeTypeForUrl(url);
        // was:  d->m_mimeType = KMimeType::findByUrl( url, d->m_fileMode, isLocalUrl );
        // => we are no longer using d->m_fileMode for remote URLs.
        Q_ASSERT(d->m_mimeType.isValid());
        //qDebug() << d << "finding final mimetype for" << url << ":" << d->m_mimeType.name();
        d->m_bMimeTypeKnown = true;
    }

    return d->m_mimeType;
}

bool KFileItem::isMimeTypeKnown() const
{
    if (!d)
        return false;

    // The mimetype isn't known if determineMimeType was never called (on-demand determination)
    // or if this fileitem has a guessed mimetype (e.g. ftp symlink) - in which case
    // it always remains "not fully determined"
    return d->m_bMimeTypeKnown && d->m_guessedMimeType.isEmpty();
}

static bool isDirectoryMounted(const QUrl& url)
{
    // Stating .directory files can cause long freezes when e.g. /home
    // uses autofs for every user's home directory, i.e. opening /home
    // in a file dialog will mount every single home directory.
    // These non-mounted directories can be identified by having 0 size.
    // There are also other directories with 0 size, such as /proc, that may
    // be mounted, but those are unlikely to contain .directory (and checking
    // this would require checking with KMountPoint).

    // TODO: maybe this could be checked with KFileSystemType instead?
    KDE_struct_stat buff;
    if (KDE_stat(QFile::encodeName(url.toLocalFile()), &buff) == 0
        && S_ISDIR(buff.st_mode) && buff.st_size == 0) {
        return false;
    }
    return true;
}

// KDE5 TODO: merge with comment()? Need to see what lxr says about the usage of both.
QString KFileItem::mimeComment() const
{
    if (!d)
        return QString();

    const QString displayType = d->m_entry.stringValue( KIO::UDSEntry::UDS_DISPLAY_TYPE );
    if (!displayType.isEmpty())
        return displayType;

    bool isLocalUrl;
    QUrl url = mostLocalUrl(isLocalUrl);

    QMimeType mime = currentMimeType();
    // This cannot move to kio_file (with UDS_DISPLAY_TYPE) because it needs
    // the mimetype to be determined, which is done here, and possibly delayed...
    if (isLocalUrl && !d->isSlow() && mime.inherits("application/x-desktop")) {
        KDesktopFile cfg( url.toLocalFile() );
        QString comment = cfg.desktopGroup().readEntry( "Comment" );
        if (!comment.isEmpty())
            return comment;
    }

    // Support for .directory file in directories
    if (isLocalUrl && isDir() && isDirectoryMounted(url)) {
        QUrlPathInfo u(url);
        u.addPath(QString::fromLatin1(".directory"));
        const KDesktopFile cfg(u.localPath());
        const QString comment = cfg.readComment();
        if (!comment.isEmpty())
            return comment;
    }

    const QString comment = mime.comment();
    //kDebug() << "finding comment for " << url.url() << " : " << d->m_mimeType->name();
    if (!comment.isEmpty())
        return comment;
    else
        return mime.name();
}

static QString iconFromDirectoryFile(const QString& path)
{
    QUrlPathInfo u(QUrl::fromLocalFile(path));
    u.addPath(QString::fromLatin1(".directory"));
    const QString filePath = u.localPath();
    if (!QFileInfo(filePath).isFile()) // exists -and- is a file
        return QString();

    KDesktopFile cfg(filePath);
    QString icon = cfg.readIcon();

    const KConfigGroup group = cfg.desktopGroup();
    const QString emptyIcon = group.readEntry( "EmptyIcon" );
    if (!emptyIcon.isEmpty()) {
        bool isDirEmpty = true;
        QDirIterator dirIt(path, QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot);
        while (dirIt.hasNext()) {
            dirIt.next();
            if (dirIt.fileName() != QLatin1String(".directory")) {
                isDirEmpty = false;
                break;
            }
        }
        if (isDirEmpty) {
            icon = emptyIcon;
        }
    }

    if (icon.startsWith(QLatin1String("./"))) {
        // path is relative with respect to the location
        // of the .directory file (#73463)
        QUrlPathInfo iconUrl(QUrl::fromLocalFile(path));
        iconUrl.addPath(icon.mid(2));
        return iconUrl.localPath();
    }
    return icon;
}

static QString iconFromDesktopFile(const QString& path)
{
    KDesktopFile cfg(path);
    const QString icon = cfg.readIcon();
    if (cfg.hasLinkType()) {
        const KConfigGroup group = cfg.desktopGroup();
        const QString emptyIcon = group.readEntry( "EmptyIcon" );
        const QString type = cfg.readPath();
        if ( !emptyIcon.isEmpty() ) {
            const QString u = cfg.readUrl();
            const QUrl url(u);
            if (url.scheme() == QLatin1String("trash")) {
                // We need to find if the trash is empty, preferably without using a KIO job.
                // So instead kio_trash leaves an entry in its config file for us.
                KConfig trashConfig( "trashrc", KConfig::SimpleConfig );
                if ( trashConfig.group("Status").readEntry( "Empty", true ) ) {
                    return emptyIcon;
                }
            }
        }
    }
    return icon;
}

QString KFileItem::iconName() const
{
    if (!d)
        return QString();

    if (d->m_useIconNameCache && !d->m_iconName.isEmpty()) {
        return d->m_iconName;
    }

    d->m_iconName = d->m_entry.stringValue( KIO::UDSEntry::UDS_ICON_NAME );
    if (!d->m_iconName.isEmpty()) {
        d->m_useIconNameCache = d->m_bMimeTypeKnown;
        return d->m_iconName;
    }

    bool isLocalUrl;
    QUrl url = mostLocalUrl(isLocalUrl);

    QMimeDatabase db;
    QMimeType mime;
    // Use guessed mimetype for the icon
    if (!d->m_guessedMimeType.isEmpty()) {
        mime = db.mimeTypeForName(d->m_guessedMimeType);
    } else {
        mime = currentMimeType();
    }

    if (isLocalUrl && !isSlow() && mime.inherits("application/x-desktop")) {
        d->m_iconName = iconFromDesktopFile(url.toLocalFile());
        if (!d->m_iconName.isEmpty()) {
            d->m_useIconNameCache = d->m_bMimeTypeKnown;
            return d->m_iconName;
        }
    }

    if (isLocalUrl && isDir() && isDirectoryMounted(url)) {
        d->m_iconName = iconFromDirectoryFile(url.toLocalFile());
        if (!d->m_iconName.isEmpty()) {
            d->m_useIconNameCache = d->m_bMimeTypeKnown;
            return d->m_iconName;
        }
    }

    d->m_iconName = mime.iconName();
    d->m_useIconNameCache = d->m_bMimeTypeKnown;
    return d->m_iconName;
}

/**
 * Returns true if this is a desktop file.
 * Mimetype determination is optional.
 */
static bool checkDesktopFile(const KFileItem& item, bool _determineMimeType)
{
    // only local files
    bool isLocal;
    const QUrl url = item.mostLocalUrl(isLocal);
    if (!isLocal)
        return false;

    // only regular files
    if (!item.isRegularFile())
        return false;

    // only if readable
    if (!item.isReadable())
        return false;

    // return true if desktop file
    QMimeType mime = _determineMimeType ? item.determineMimeType() : item.currentMimeType();
    return mime.inherits("application/x-desktop");
}

QStringList KFileItem::overlays() const
{
    if (!d)
        return QStringList();

    QStringList names = d->m_entry.stringValue( KIO::UDSEntry::UDS_ICON_OVERLAY_NAMES ).split(',');
    if ( d->m_bLink ) {
        names.append("emblem-symbolic-link");
    }

    if ( !S_ISDIR( d->m_fileMode ) // Locked dirs have a special icon, use the overlay for files only
         && !isReadable()) {
        names.append("object-locked");
    }

    if ( checkDesktopFile(*this, false) ) {
        KDesktopFile cfg( localPath() );
        const KConfigGroup group = cfg.desktopGroup();

        // Add a warning emblem if this is an executable desktop file
        // which is untrusted.
        if ( group.hasKey( "Exec" ) && !KDesktopFile::isAuthorizedDesktopFile( localPath() ) ) {
            names.append( "emblem-important" );
        }

        if (cfg.hasDeviceType()) {
            const QString dev = cfg.readDevice();
            if (!dev.isEmpty()) {
                KMountPoint::Ptr mountPoint = KMountPoint::currentMountPoints().findByDevice(dev);
                if (mountPoint) // mounted?
                    names.append("emblem-mounted");
            }
        }
    }

    if ( isHidden() ) {
        names.append("hidden");
    }

#ifndef Q_OS_WIN
    if( S_ISDIR( d->m_fileMode ) && d->m_bIsLocalUrl)
    {
        if (KSambaShare::instance()->isDirectoryShared( d->m_url.toLocalFile() ) ||
            KNFSShare::instance()->isDirectoryShared( d->m_url.toLocalFile() ))
        {
            //kDebug() << d->m_url.path();
            names.append("network-workgroup");
        }
    }
#endif  // Q_OS_WIN

    if (d->m_url.path().endsWith(QLatin1String(".gz")) &&
        d->m_mimeType.inherits("application/x-gzip")) {
        names.append("application-zip");
    }

    return names;
}

QString KFileItem::comment() const
{
    if (!d)
        return QString();

    return d->m_entry.stringValue( KIO::UDSEntry::UDS_COMMENT );
}

// ## where is this used?
QPixmap KFileItem::pixmap( int _size, int _state ) const
{
    if (!d)
        return QPixmap();

    const QString udsIconName = d->m_entry.stringValue( KIO::UDSEntry::UDS_ICON_NAME );
    if ( !udsIconName.isEmpty() )
        return DesktopIcon(udsIconName, _size, _state);

    QMimeDatabase db;

    if (!d->m_useIconNameCache && !d->m_mimeType.isValid()) {
        // No mimetype determined yet, go for a fast default icon
        if (S_ISDIR(d->m_fileMode)) {
            static QString defaultFolderIcon;
            if ( defaultFolderIcon.isEmpty() ) {
                const QMimeType mimeType = db.mimeTypeForName("inode/directory");
                if (mimeType.isValid())
                    defaultFolderIcon = mimeType.iconName();
                else {
                    kWarning(7000) << "No mimetype for inode/directory could be found. Check your installation.";
                    defaultFolderIcon = "unknown";
                }
            }
            return DesktopIcon( defaultFolderIcon, _size, _state );
        }
        return DesktopIcon( "unknown", _size, _state );
    }

    QMimeType mime;
    // Use guessed mimetype for the icon
    if (!d->m_guessedMimeType.isEmpty())
        mime = db.mimeTypeForName(d->m_guessedMimeType);
    else
        mime = d->m_mimeType;

    QString icon = iconName();

    // Support for gzipped files: extract mimetype of contained file
    // See also the relevant code in overlays, which adds the zip overlay.
    if (mime.name() == "application/x-gzip" && d->m_url.path().endsWith(QLatin1String(".gz"))) {
        QUrl sf(d->m_url);
        sf.setPath(sf.path().left(sf.path().length() - 3));
        //kDebug() << "subFileName=" << subFileName;
        mime = db.mimeTypeForUrl(sf);
        icon = mime.iconName();
    }

    QPixmap p = KIconLoader::global()->loadMimeTypeIcon(icon, KIconLoader::Desktop, _size, _state);
    //kDebug() << "finding pixmap for" << url << "mime=" << mime.name() << "icon=" << icon;
    if (p.isNull())
        kWarning() << "Pixmap not found for mimetype" << mime.name() << "icon" << icon;

    return p;
}

bool KFileItem::isReadable() const
{
    if (!d)
        return false;

    /*
      struct passwd * user = getpwuid( geteuid() );
      bool isMyFile = (QString::fromLocal8Bit(user->pw_name) == d->m_user);
      // This gets ugly for the group....
      // Maybe we want a static QString for the user and a static QStringList
      // for the groups... then we need to handle the deletion properly...
      */

    if (d->m_permissions != KFileItem::Unknown) {
        // No read permission at all
        if ( !(S_IRUSR & d->m_permissions) && !(S_IRGRP & d->m_permissions) && !(S_IROTH & d->m_permissions) )
            return false;

        // Read permissions for all: save a stat call
        if ( (S_IRUSR|S_IRGRP|S_IROTH) & d->m_permissions )
            return true;
    }

    // Or if we can't read it [using ::access()] - not network transparent
    if ( d->m_bIsLocalUrl && KDE::access( d->m_url.toLocalFile(), R_OK ) == -1 )
        return false;

    return true;
}

bool KFileItem::isWritable() const
{
    if (!d)
        return false;

    /*
      struct passwd * user = getpwuid( geteuid() );
      bool isMyFile = (QString::fromLocal8Bit(user->pw_name) == d->m_user);
      // This gets ugly for the group....
      // Maybe we want a static QString for the user and a static QStringList
      // for the groups... then we need to handle the deletion properly...
      */

    if (d->m_permissions != KFileItem::Unknown) {
        // No write permission at all
        if ( !(S_IWUSR & d->m_permissions) && !(S_IWGRP & d->m_permissions) && !(S_IWOTH & d->m_permissions) )
            return false;
    }

    // Or if we can't read it [using ::access()] - not network transparent
    if ( d->m_bIsLocalUrl && KDE::access( d->m_url.toLocalFile(), W_OK ) == -1 )
        return false;

    return true;
}

bool KFileItem::isHidden() const
{
    if (!d)
        return false;

    // The kioslave can specify explicitly that a file is hidden or shown
    if ( d->m_hidden != KFileItemPrivate::Auto )
        return d->m_hidden == KFileItemPrivate::Hidden;

    // Prefer the filename that is part of the URL, in case the display name is different.
    QString fileName = QUrlPathInfo(d->m_url).fileName();
    if (fileName.isEmpty()) // e.g. "trash:/"
        fileName = d->m_strName;
    return fileName.length() > 1 && fileName[0] == '.';  // Just "." is current directory, not hidden.
}

bool KFileItem::isDir() const
{
    if (!d)
        return false;

    if (d->m_fileMode == KFileItem::Unknown) {
        // Probably the file was deleted already, and KDirLister hasn't told the world yet.
        //kDebug() << d << url() << "can't say -> false";
        return false; // can't say for sure, so no
    }
    return (S_ISDIR(d->m_fileMode));
}

bool KFileItem::isFile() const
{
    if (!d)
        return false;

    return !isDir();
}

#ifndef KDE_NO_DEPRECATED
bool KFileItem::acceptsDrops() const
{
    // A directory ?
    if ( S_ISDIR( mode() ) ) {
        return isWritable();
    }

    // But only local .desktop files and executables
    if ( !d->m_bIsLocalUrl )
        return false;

    if ( mimetype() == "application/x-desktop")
        return true;

    // Executable, shell script ... ?
    if ( QFileInfo(d->m_url.toLocalFile()).isExecutable() )
        return true;

    return false;
}
#endif

QString KFileItem::getStatusBarInfo() const
{
    if (!d)
        return QString();

    QString text = d->m_strText;
    const QString comment = mimeComment();

    if ( d->m_bLink )
    {
        text += ' ';
        if ( comment.isEmpty() )
            text += i18n ( "(Symbolic Link to %1)", linkDest() );
        else
            text += i18n("(%1, Link to %2)", comment, linkDest());
    }
    else if ( targetUrl() != url() )
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        text += i18n(" (Points to %1)", targetUrl().toString());
#else
        text += i18n(" (Points to %1)", targetUrl().toDisplayString());
#endif
    }
    else if ( S_ISREG( d->m_fileMode ) )
    {
        text += QString(" (%1, %2)").arg( comment, KIO::convertSize( size() ) );
    }
    else
    {
        text += QString(" (%1)").arg( comment );
    }
    return text;
}

#ifndef KDE_NO_DEPRECATED
QString KFileItem::getToolTipText(int maxcount) const
{
    if (!d)
        return QString();

    // we can return QString() if no tool tip should be shown
    QString tip;
    KFileMetaInfo info = metaInfo();

    // the font tags are a workaround for the fact that the tool tip gets
    // screwed if the color scheme uses white as default text color
    const QString colorName = QApplication::palette().color(QPalette::ToolTipText).name();
    const QString start = "<tr><td align=\"right\"><nobr><font color=\"" + colorName + "\"><b>";
    const QString mid = "&nbsp;</b></font></nobr></td><td><nobr><font color=\"" + colorName + "\">";
    const char* end = "</font></nobr></td></tr>";

    tip = "<table cellspacing=0 cellpadding=0>";

    tip += start + i18n("Name:") + mid + text() + end;
    tip += start + i18n("Type:") + mid;

    QString type = Qt::escape(mimeComment());
    if ( d->m_bLink ) {
        tip += i18n("Link to %1 (%2)", linkDest(), type) + end;
    } else
        tip += type + end;

    if ( !S_ISDIR ( d->m_fileMode ) )
        tip += start + i18n("Size:") + mid +
               QString("%1").arg(KIO::convertSize(size())) +
               end;

    tip += start + i18n("Modified:") + mid +
           timeString( KFileItem::ModificationTime ) + end
#ifndef Q_OS_WIN //TODO: show win32-specific permissions
           +start + i18n("Owner:") + mid + user() + " - " + group() + end +
           start + i18n("Permissions:") + mid +
           permissionsString() + end
#endif
           ;

    if (info.isValid())
    {
        const QStringList keys = info.preferredKeys();

        // now the rest
        QStringList::ConstIterator it = keys.begin();
        for (int count = 0; count<maxcount && it!=keys.end() ; ++it)
        {
            if ( count == 0 )
            {
                tip += "<tr><td colspan=2><center><s>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</s></center></td></tr>";
            }

            KFileMetaInfoItem item = info.item( *it );
            if ( item.isValid() )
            {
                QString s = item.value().toString();
                if ( !s.isEmpty() )
                {
                    count++;
                    tip += start +
                           Qt::escape( item.name() ) + ':' +
                           mid +
                           Qt::escape( s ) +
                           end;
                }

            }
        }
    }
    tip += "</table>";

    //kDebug() << "making this the tool tip rich text:\n";
    //kDebug() << tip;

    return tip;
}
#endif

void KFileItem::run( QWidget* parentWidget ) const
{
    if (!d) {
        kWarning() << "null item";
        return;
    }

    (void) new KRun( targetUrl(), parentWidget, d->m_fileMode, d->m_bIsLocalUrl );
}

bool KFileItem::cmp( const KFileItem & item ) const
{
    if (!d && !item.d)
        return true;

    if (!d || !item.d)
        return false;

    return d->cmp(*item.d);
}

bool KFileItem::operator==(const KFileItem& other) const
{
    if (!d && !other.d)
        return true;

    if (!d || !other.d)
        return false;

    return d->m_url == other.d->m_url;
}

bool KFileItem::operator!=(const KFileItem& other) const
{
    return !operator==(other);
}

KFileItem::operator QVariant() const
{
    return qVariantFromValue(*this);
}

#ifndef KDE_NO_DEPRECATED
void KFileItem::setExtraData( const void *key, void *value )
{
    if (!d)
        return;

    if ( !key )
        return;

    d->m_extra.insert( key, value ); // replaces the value of key if already there
}
#endif

#ifndef KDE_NO_DEPRECATED
const void * KFileItem::extraData( const void *key ) const
{
    if (!d)
        return 0;

    return d->m_extra.value( key, 0 );
}
#endif

#ifndef KDE_NO_DEPRECATED
void KFileItem::removeExtraData( const void *key )
{
    if (!d)
        return;

    d->m_extra.remove( key );
}
#endif

QString KFileItem::permissionsString() const
{
    if (!d)
        return QString();

    if (d->m_access.isNull() && d->m_permissions != KFileItem::Unknown)
        d->m_access = d->parsePermissions( d->m_permissions );

    return d->m_access;
}

// check if we need to cache this
QString KFileItem::timeString( FileTimes which ) const
{
    if (!d)
        return QString();

    return KLocale::global()->formatDateTime( d->time(which) );
}

#ifndef KDE_NO_DEPRECATED
QString KFileItem::timeString( unsigned int which ) const
{
    if (!d)
        return QString();

    switch (which) {
    case KIO::UDSEntry::UDS_ACCESS_TIME:
        return timeString(AccessTime);
    case KIO::UDSEntry::UDS_CREATION_TIME:
        return timeString(CreationTime);
    case KIO::UDSEntry::UDS_MODIFICATION_TIME:
    default:
        return timeString(ModificationTime);
    }
}
#endif

void KFileItem::setMetaInfo( const KFileMetaInfo & info ) const
{
    if (!d)
        return;

    d->m_metaInfo = info;
}

KFileMetaInfo KFileItem::metaInfo(bool autoget, int what) const
{
    if (!d)
        return KFileMetaInfo();

    if ((isRegularFile() || isDir()) && autoget && !d->m_metaInfo.isValid())
    {
        bool isLocalUrl;
        QUrl url(mostLocalUrl(isLocalUrl));
        if (isLocalUrl) {
            d->m_metaInfo = KFileMetaInfo(url.toLocalFile(), mimetype(), (KFileMetaInfo::What)what);
        }
    }
    return d->m_metaInfo;
}

#ifndef KDE_NO_DEPRECATED
void KFileItem::assign( const KFileItem & item )
{
    *this = item;
}
#endif

QUrl KFileItem::mostLocalUrl(bool &local) const
{
    if (!d)
        return QUrl();

    const QString local_path = localPath();
    if ( !local_path.isEmpty() ) {
        local = true;
        return QUrl::fromLocalFile(local_path);
    } else {
        local = d->m_bIsLocalUrl;
        return d->m_url;
    }
}

QUrl KFileItem::mostLocalUrl() const
{
    bool local = false;
    return mostLocalUrl(local);
}

QDataStream & operator<< ( QDataStream & s, const KFileItem & a )
{
    if (a.d) {
        // We don't need to save/restore anything that refresh() invalidates,
        // since that means we can re-determine those by ourselves.
        s << a.d->m_url;
        s << a.d->m_strName;
        s << a.d->m_strText;
    } else {
        s << QUrl();
        s << QString();
        s << QString();
    }

    return s;
}

QDataStream & operator>> ( QDataStream & s, KFileItem & a )
{
    QUrl url;
    QString strName, strText;

    s >> url;
    s >> strName;
    s >> strText;

    if (!a.d) {
        kWarning() << "null item";
        return s;
    }

    if (url.isEmpty()) {
        a.d = 0;
        return s;
    }

    a.d->m_url = url;
    a.d->m_strName = strName;
    a.d->m_strText = strText;
    a.d->m_bIsLocalUrl = a.d->m_url.isLocalFile();
    a.d->m_bMimeTypeKnown = false;
    a.refresh();

    return s;
}

QUrl KFileItem::url() const
{
    if (!d)
        return QUrl();

    return d->m_url;
}

mode_t KFileItem::permissions() const
{
    if (!d)
        return 0;

    return d->m_permissions;
}

mode_t KFileItem::mode() const
{
    if (!d)
        return 0;

    return d->m_fileMode;
}

bool KFileItem::isLink() const
{
    if (!d)
        return false;

    return d->m_bLink;
}

bool KFileItem::isLocalFile() const
{
    if (!d)
        return false;

    return d->m_bIsLocalUrl;
}

QString KFileItem::text() const
{
    if (!d)
        return QString();

    return d->m_strText;
}

QString KFileItem::name( bool lowerCase ) const
{
    if (!d)
        return QString();

    if ( !lowerCase )
        return d->m_strName;
    else
        if ( d->m_strLowerCaseName.isNull() )
            d->m_strLowerCaseName = d->m_strName.toLower();
    return d->m_strLowerCaseName;
}

QUrl KFileItem::targetUrl() const
{
    if (!d)
        return QUrl();

    const QString targetUrlStr = d->m_entry.stringValue( KIO::UDSEntry::UDS_TARGET_URL );
    if (!targetUrlStr.isEmpty())
        return QUrl(targetUrlStr);
    else
        return url();
}

QUrl KFileItem::nepomukUri() const
{
#if ! KIO_NO_NEPOMUK
    if (!d)
        return QUrl();

    const QString nepomukUriStr = d->m_entry.stringValue( KIO::UDSEntry::UDS_NEPOMUK_URI );
    if(!nepomukUriStr.isEmpty()) {
        return QUrl(nepomukUriStr);
    }
    else if(targetUrl().isLocalFile()) {
        return targetUrl();
    }
    else {
        return QUrl();
    }
#else
    return QUrl();
#endif
}

/*
 * Mimetype handling.
 *
 * Initial state: m_mimeType = QMimeType().
 * When currentMimeType() is called first: fast mimetype determination,
 *   might either find an accurate mimetype (-> Final state), otherwise we
 *   set m_mimeType but not m_bMimeTypeKnown (-> Intermediate state)
 * Intermediate state: determineMimeType() does the real determination -> Final state.
 *
 * If delayedMimeTypes isn't set, then we always go to the Final state directly.
 */

QMimeType KFileItem::currentMimeType() const
{
    if (!d)
        return QMimeType();

    if (!d->m_mimeType.isValid()) {
        // On-demand fast (but not always accurate) mimetype determination
        Q_ASSERT(!d->m_url.isEmpty());
        QMimeDatabase db;
        const QUrl url = mostLocalUrl();
        if (d->m_delayedMimeTypes) {
            const QList<QMimeType> mimeTypes = db.mimeTypesForFileName(url.path());
            if (mimeTypes.isEmpty()) {
                d->m_mimeType = db.mimeTypeForName("application/octet-stream");
                d->m_bMimeTypeKnown = false;
            } else {
                d->m_mimeType = mimeTypes.first();
                // If there were conflicting globs. determineMimeType will be able to do better.
                d->m_bMimeTypeKnown = (mimeTypes.count() == 1);
            }
        } else {
            // ## d->m_fileMode isn't used anymore (for remote urls)
            d->m_mimeType = db.mimeTypeForUrl(url);
            d->m_bMimeTypeKnown = true;
        }
    }
    return d->m_mimeType;
}

KIO::UDSEntry KFileItem::entry() const
{
    if (!d)
        return KIO::UDSEntry();

    return d->m_entry;
}

bool KFileItem::isMarked() const
{
    if (!d)
        return false;

    return d->m_bMarked;
}

void KFileItem::mark()
{
    if (!d) {
        kWarning() << "null item";
        return;
    }

    d->m_bMarked = true;
}

void KFileItem::unmark()
{
    if (!d) {
        kWarning() << "null item";
        return;
    }

    d->m_bMarked = false;
}

KFileItem& KFileItem::operator=(const KFileItem& other)
{
    d = other.d;
    return *this;
}

bool KFileItem::isNull() const
{
    return d == 0;
}

KFileItemList::KFileItemList()
{
}

KFileItemList::KFileItemList( const QList<KFileItem> &items )
  : QList<KFileItem>( items )
{
}

KFileItem KFileItemList::findByName( const QString& fileName ) const
{
    const_iterator it = begin();
    const const_iterator itend = end();
    for ( ; it != itend ; ++it ) {
        if ( (*it).name() == fileName ) {
            return *it;
        }
    }
    return KFileItem();
}

KFileItem KFileItemList::findByUrl(const QUrl& url) const
{
    const_iterator it = begin();
    const const_iterator itend = end();
    for ( ; it != itend ; ++it ) {
        if ( (*it).url() == url ) {
            return *it;
        }
    }
    return KFileItem();
}

QList<QUrl> KFileItemList::urlList() const {
    QList<QUrl> lst;
    const_iterator it = begin();
    const const_iterator itend = end();
    for ( ; it != itend ; ++it ) {
        lst.append( (*it).url() );
    }
    return lst;
}

QList<QUrl> KFileItemList::targetUrlList() const {
    QList<QUrl> lst;
    const_iterator it = begin();
    const const_iterator itend = end();
    for ( ; it != itend ; ++it ) {
        lst.append( (*it).targetUrl() );
    }
    return lst;
}


bool KFileItem::isDesktopFile() const
{
    return checkDesktopFile(*this, true);
}

bool KFileItem::isRegularFile() const
{
    if (!d)
        return false;

    return S_ISREG(d->m_fileMode);
}

QDebug operator<<(QDebug stream, const KFileItem& item)
{
    if (item.isNull()) {
        stream << "[null KFileItem]";
    } else {
        stream << "[KFileItem for" << item.url() << "]";
    }
    return stream;
}
