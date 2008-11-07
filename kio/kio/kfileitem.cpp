/* This file is part of the KDE project
   Copyright (C) 1999-2006 David Faure <faure@kde.org>
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

#include <config.h>

#include <sys/time.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>
#include <unistd.h>

#include <QtCore/QDate>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QMap>
#include <QTextDocument>

#include <kdebug.h>
#include <kfilemetainfo.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmimetype.h>
#include <krun.h>
#include <kde_file.h>
#include <kdesktopfile.h>
#include <kmountpoint.h>
#include <kconfiggroup.h>
#ifndef Q_OS_WIN
#include <knfsshare.h>
#include <ksambashare.h>
#endif

class KFileItemPrivate : public QSharedData
{
public:
    KFileItemPrivate(const KIO::UDSEntry& entry,
                     mode_t mode, mode_t permissions,
                     const KUrl& url, // item url if entry.isEmpty(), otherwise directory url
                     bool urlIsDirectory,
                     bool delayedMimeTypes)
        : m_entry( entry ),
          m_url( url ),
          m_strName(),
          m_strText(),
          m_iconName(),
          m_strLowerCaseName(),
          m_pMimeType( 0 ),
          m_fileMode( mode ),
          m_permissions( permissions ),
          m_bMarked( false ),
          m_bLink( false ),
          m_bIsLocalUrl( url.isLocalFile() ),
          m_bMimeTypeKnown( false ),
          m_delayedMimeTypes( delayedMimeTypes ),
          m_useIconNameCache(false),
          m_hidden( Auto )
    {
        if (entry.count() != 0) {
            readUDSEntry( urlIsDirectory );
        } else {
            m_strName = url.fileName();
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

    KIO::filesize_t size() const;
    KDateTime time( KFileItem::FileTimes which ) const;
    void setTime(KFileItem::FileTimes which, long long time_t_val) const;
    bool cmp( const KFileItemPrivate & item ) const;

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
    KUrl m_url;

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
    mutable KMimeType::Ptr m_pMimeType;

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

    // For special case like link to dirs over FTP
    QString m_guessedMimeType;
    mutable QString m_access;
    QMap<const void*, void*> m_extra; // DEPRECATED
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
            const QByteArray path = QFile::encodeName(m_url.path( KUrl::RemoveTrailingSlash ));
            if ( KDE_lstat( path.data(), &buf ) == 0 )
            {
                mode = buf.st_mode;
                if ( S_ISLNK( mode ) )
                {
                    m_bLink = true;
                    if ( KDE_stat( path.data(), &buf ) == 0 )
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
        m_url = KUrl( urlStr );
        if ( m_url.isLocalFile() )
            m_bIsLocalUrl = true;
    }
    const QString mimeTypeStr = m_entry.stringValue( KIO::UDSEntry::UDS_MIME_TYPE );
    m_bMimeTypeKnown = !mimeTypeStr.isEmpty();
    if ( m_bMimeTypeKnown )
        m_pMimeType = KMimeType::mimeType( mimeTypeStr );

    m_guessedMimeType = m_entry.stringValue( KIO::UDSEntry::UDS_GUESSED_MIME_TYPE );
    m_bLink = !m_entry.stringValue( KIO::UDSEntry::UDS_LINK_DEST ).isEmpty(); // we don't store the link dest

    const int hiddenVal = m_entry.numberValue( KIO::UDSEntry::UDS_HIDDEN, -1 );
    m_hidden = hiddenVal == 1 ? Hidden : ( hiddenVal == 0 ? Shown : Auto );

    // avoid creating these QStrings again and again
    static const QString& dot = KGlobal::staticQString(".");
    if ( _urlIsDirectory && !UDS_URL_seen && !m_strName.isEmpty() && m_strName != dot )
        m_url.addPath( m_strName );

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
        if ( KDE_stat( QFile::encodeName(m_url.path(KUrl::RemoveTrailingSlash)), &buf ) == 0 )
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
        if ( KDE_stat( QFile::encodeName(m_url.path(KUrl::RemoveTrailingSlash)), &buf ) == 0 )
        {
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
    return ( m_strName == item.m_strName
             && m_bIsLocalUrl == item.m_bIsLocalUrl
             && m_fileMode == item.m_fileMode
             && m_permissions == item.m_permissions
             && m_entry.stringValue( KIO::UDSEntry::UDS_USER ) == item.m_entry.stringValue( KIO::UDSEntry::UDS_USER )
             && m_entry.stringValue( KIO::UDSEntry::UDS_GROUP ) == item.m_entry.stringValue( KIO::UDSEntry::UDS_GROUP )
             && m_entry.stringValue( KIO::UDSEntry::UDS_EXTENDED_ACL ) == item.m_entry.stringValue( KIO::UDSEntry::UDS_EXTENDED_ACL )
             && m_entry.stringValue( KIO::UDSEntry::UDS_ACL_STRING ) == item.m_entry.stringValue( KIO::UDSEntry::UDS_ACL_STRING )
             && m_entry.stringValue( KIO::UDSEntry::UDS_DEFAULT_ACL_STRING ) == item.m_entry.stringValue( KIO::UDSEntry::UDS_DEFAULT_ACL_STRING )
             && m_bLink == item.m_bLink
             && m_hidden == item.m_hidden
             && size() == item.size()
             && time(KFileItem::ModificationTime) == item.time(KFileItem::ModificationTime)
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
    if (m_fileMode != KFileItem::Unknown && S_ISDIR(m_fileMode))
        buffer[0] = 'd';
    else if (m_bLink)
        buffer[0] = 'l';
    else
        buffer[0] = '-';

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

KFileItem::KFileItem( const KIO::UDSEntry& entry, const KUrl& directoryUrl,
                      bool delayedMimeTypes, bool urlIsDirectory )
    : d(new KFileItemPrivate(entry, KFileItem::Unknown, KFileItem::Unknown,
                             directoryUrl, urlIsDirectory, delayedMimeTypes))
{
}

KFileItem::KFileItem( mode_t mode, mode_t permissions, const KUrl& url, bool delayedMimeTypes )
    : d(new KFileItemPrivate(KIO::UDSEntry(), mode, permissions,
                             url, false, delayedMimeTypes))
{
}

KFileItem::KFileItem( const KUrl &url, const QString &mimeType, mode_t mode )
    : d(new KFileItemPrivate(KIO::UDSEntry(), mode, KFileItem::Unknown,
                             url, false, false))
{
    d->m_bMimeTypeKnown = !mimeType.isEmpty();
    if (d->m_bMimeTypeKnown)
        d->m_pMimeType = KMimeType::mimeType( mimeType );
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
    d->m_pMimeType = 0;
    d->m_bMimeTypeKnown = false;
    d->m_iconName.clear();
}

void KFileItem::setUrl( const KUrl &url )
{
    d->m_url = url;
    setName( url.fileName() );
}

void KFileItem::setName( const QString& name )
{
    d->m_strName = name;
    d->m_strText = KIO::decodeFileName( d->m_strName );
}

QString KFileItem::linkDest() const
{
    // Extract it from the KIO::UDSEntry
    const QString linkStr = d->m_entry.stringValue( KIO::UDSEntry::UDS_LINK_DEST );
    if ( !linkStr.isEmpty() )
        return linkStr;

    // If not in the KIO::UDSEntry, or if UDSEntry empty, use readlink() [if local URL]
    if ( d->m_bIsLocalUrl )
    {
        char buf[1000];
        int n = readlink( QFile::encodeName(d->m_url.path( KUrl::RemoveTrailingSlash )), buf, sizeof(buf)-1 );
        if ( n != -1 )
        {
            buf[ n ] = 0;
            return QFile::decodeName( buf );
        }
    }
    return QString();
}

QString KFileItem::localPath() const
{
  if ( d->m_bIsLocalUrl ) {
    return d->m_url.path();
  }

  // Extract the local path from the KIO::UDSEntry
  return d->m_entry.stringValue( KIO::UDSEntry::UDS_LOCAL_PATH );
}

KIO::filesize_t KFileItem::size() const
{
    return d->size();
}

bool KFileItem::hasExtendedACL() const
{
    // Check if the field exists; its value doesn't matter
    return d->m_entry.contains(KIO::UDSEntry::UDS_EXTENDED_ACL);
}

KACL KFileItem::ACL() const
{
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
    // Extract it from the KIO::UDSEntry
    const QString fieldVal = d->m_entry.stringValue( KIO::UDSEntry::UDS_DEFAULT_ACL_STRING );
    if ( !fieldVal.isEmpty() )
        return KACL(fieldVal);
    else
        return KACL();
}

KDateTime KFileItem::time( FileTimes which ) const
{
    return d->time(which);
}

time_t KFileItem::time( unsigned int which ) const
{
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

QString KFileItem::user() const
{
    QString userName = d->m_entry.stringValue( KIO::UDSEntry::UDS_USER );
    if ( userName.isEmpty() && d->m_bIsLocalUrl )
    {
#ifdef Q_WS_WIN
        QFileInfo a(d->m_url.path( KUrl::RemoveTrailingSlash ));
        userName = a.owner();
        d->m_entry.insert( KIO::UDSEntry::UDS_USER, userName );
#else
        KDE_struct_stat buff;
        if ( KDE_lstat( QFile::encodeName(d->m_url.path( KUrl::RemoveTrailingSlash )), &buff ) == 0) // get uid/gid of the link, if it's a link
        {
            struct passwd *pwuser = getpwuid( buff.st_uid );
            if ( pwuser != 0 ) {
                userName = QString::fromLocal8Bit(pwuser->pw_name);
                d->m_entry.insert( KIO::UDSEntry::UDS_USER, userName );
            }
        }
#endif
    }
    return userName;
}

QString KFileItem::group() const
{
    QString groupName = d->m_entry.stringValue( KIO::UDSEntry::UDS_GROUP );
    if (groupName.isEmpty() && d->m_bIsLocalUrl )
    {
#ifdef Q_WS_WIN
        QFileInfo a(d->m_url.path( KUrl::RemoveTrailingSlash ));
        groupName = a.group();
        d->m_entry.insert( KIO::UDSEntry::UDS_GROUP, groupName );
#else
        KDE_struct_stat buff;
        if ( KDE_lstat( QFile::encodeName(d->m_url.path( KUrl::RemoveTrailingSlash )), &buff ) == 0) // get uid/gid of the link, if it's a link
        {
            struct group *ge = getgrgid( buff.st_gid );
            if ( ge != 0 ) {
                groupName = QString::fromLocal8Bit(ge->gr_name);
                if (groupName.isEmpty())
                    groupName.sprintf("%d",ge->gr_gid);
            }
            else
                groupName.sprintf("%d",buff.st_gid);
            d->m_entry.insert( KIO::UDSEntry::UDS_GROUP, groupName );
        }
#endif
    }
    return groupName;
}

QString KFileItem::mimetype() const
{
    KFileItem * that = const_cast<KFileItem *>(this);
    return that->determineMimeType()->name();
}

KMimeType::Ptr KFileItem::determineMimeType() const
{
    if ( !d->m_pMimeType || !d->m_bMimeTypeKnown )
    {
        bool isLocalUrl;
        KUrl url = mostLocalUrl(isLocalUrl);

        d->m_pMimeType = KMimeType::findByUrl( url, d->m_fileMode, isLocalUrl );
        Q_ASSERT(d->m_pMimeType);
        //kDebug() << "finding mimetype for " << url.url() << " : " << d->m_pMimeType->name();
        d->m_bMimeTypeKnown = true;
    }

    return d->m_pMimeType;
}

bool KFileItem::isMimeTypeKnown() const
{
    // The mimetype isn't known if determineMimeType was never called (on-demand determination)
    // or if this fileitem has a guessed mimetype (e.g. ftp symlink) - in which case
    // it always remains "not fully determined"
    return d->m_bMimeTypeKnown && d->m_guessedMimeType.isEmpty();
}

QString KFileItem::mimeComment() const
{
    KMimeType::Ptr mType = determineMimeType();

    bool isLocalUrl;
    KUrl url = mostLocalUrl(isLocalUrl);

    KMimeType::Ptr mime = mimeTypePtr();
    if (isLocalUrl && mime->is("application/x-desktop")) {
        KDesktopFile cfg( url.path() );
        QString comment = cfg.desktopGroup().readEntry( "Comment" );
        if (!comment.isEmpty())
            return comment;
    }

    QString comment = mType->comment( url );
    //kDebug() << "finding comment for " << url.url() << " : " << d->m_pMimeType->name();
    if (!comment.isEmpty())
        return comment;
    else
        return mType->name();
}

static QString iconFromDesktopFile(const QString& path)
{
    KDesktopFile cfg( path );
    const KConfigGroup group = cfg.desktopGroup();
    const QString icon = cfg.readIcon();
    const QString type = cfg.readPath();

    if ( cfg.hasDeviceType() )
    {
        const QString unmount_icon = group.readEntry( "UnmountIcon" );
        const QString dev = cfg.readDevice();
        if ( !icon.isEmpty() && !unmount_icon.isEmpty() && !dev.isEmpty() )
        {
            KMountPoint::Ptr mountPoint = KMountPoint::currentMountPoints().findByDevice(dev);
            if (!mountPoint) // not mounted?
                return unmount_icon;
        }
    } else if ( cfg.hasLinkType() ) {
        const QString emptyIcon = group.readEntry( "EmptyIcon" );
        if ( !emptyIcon.isEmpty() ) {
            const QString u = cfg.readUrl();
            const KUrl url( u );
            if ( url.protocol() == "trash" ) {
                // We need to find if the trash is empty, preferrably without using a KIO job.
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
    if (d->m_useIconNameCache && !d->m_iconName.isEmpty()) {
        return d->m_iconName;
    }

    d->m_iconName = d->m_entry.stringValue( KIO::UDSEntry::UDS_ICON_NAME );
    if (!d->m_iconName.isEmpty()) {
        d->m_useIconNameCache = d->m_bMimeTypeKnown;
        return d->m_iconName;
    }

    bool isLocalUrl;
    KUrl url = mostLocalUrl(isLocalUrl);

    KMimeType::Ptr mime = mimeTypePtr();
    if (isLocalUrl && mime->is("application/x-desktop")) {
        d->m_iconName = iconFromDesktopFile(url.path());
        if (!d->m_iconName.isEmpty()) {
            d->m_useIconNameCache = d->m_bMimeTypeKnown;
            return d->m_iconName;
        }
    }

    //kDebug() << "finding icon for " << url.url() << " : " << mimeTypePtr()->name();
    d->m_iconName = mime->iconName(url);
    d->m_useIconNameCache = d->m_bMimeTypeKnown;
    return d->m_iconName;
}

QStringList KFileItem::overlays() const
{
    QStringList names;
    if ( d->m_bLink ) {
        names.append("emblem-symbolic-link");
    }

    if ( !S_ISDIR( d->m_fileMode ) // Locked dirs have a special icon, use the overlay for files only
         && !isReadable()) {
        names.append("object-locked");
    }

    if ( isHidden() ) {
        names.append("hidden");
    }

#ifndef Q_OS_WIN
    if( S_ISDIR( d->m_fileMode ) && d->m_bIsLocalUrl)
    {
        if (KSambaShare::instance()->isDirectoryShared( d->m_url.path() ) ||
            KNFSShare::instance()->isDirectoryShared( d->m_url.path() ))
        {
            //kDebug() << d->m_url.path();
            names.append("network-workgroup");
        }
    }
#endif  // Q_OS_WIN

    if ( d->m_pMimeType && d->m_url.fileName().endsWith( QLatin1String( ".gz" ) ) &&
         d->m_pMimeType->is("application/x-gzip") ) {
        names.append("application-zip");
    }

    return names;
}

QPixmap KFileItem::pixmap( int _size, int _state ) const
{
    const QString iconName = d->m_entry.stringValue( KIO::UDSEntry::UDS_ICON_NAME );
    if ( !iconName.isEmpty() )
        return DesktopIcon(iconName, _size, _state);

    if ( !d->m_pMimeType )
    {
        if ( S_ISDIR( d->m_fileMode ) ) {
            static const QString * defaultFolderIcon = 0;
            if ( !defaultFolderIcon ) {
                const KMimeType::Ptr mimeType = KMimeType::mimeType( "inode/directory" );
                if ( mimeType )
                    defaultFolderIcon = &KGlobal::staticQString( mimeType->iconName() );
               else
                    kWarning(7000) << "No mimetype for inode/directory could be found. Check your installation.";
            }
            if ( defaultFolderIcon )
                return DesktopIcon( *defaultFolderIcon, _size, _state );

        }
        return DesktopIcon( "unknown", _size, _state );
    }

    KMimeType::Ptr mime;
    // Use guessed mimetype if the main one hasn't been determined for sure
    if ( !d->m_bMimeTypeKnown && !d->m_guessedMimeType.isEmpty() )
        mime = KMimeType::mimeType( d->m_guessedMimeType );
    else
        mime = d->m_pMimeType;

    // Support for gzipped files: extract mimetype of contained file
    // See also the relevant code in overlays, which adds the zip overlay.
    if ( mime->name() == "application/x-gzip" && d->m_url.fileName().endsWith( QLatin1String( ".gz" ) ) )
    {
        KUrl sf;
        sf.setPath( d->m_url.path().left( d->m_url.path().length() - 3 ) );
        //kDebug() << "subFileName=" << subFileName;
        mime = KMimeType::findByUrl( sf, 0, d->m_bIsLocalUrl );
    }

    bool isLocalUrl;
    KUrl url = mostLocalUrl(isLocalUrl);

    QPixmap p = KIconLoader::global()->loadMimeTypeIcon( mime->iconName( url ), KIconLoader::Desktop, _size, _state );
    //kDebug() << "finding pixmap for " << url.url() << " : " << mime->name();
    if (p.isNull())
        kWarning() << "Pixmap not found for mimetype " << d->m_pMimeType->name();

    return p;
}

bool KFileItem::isReadable() const
{
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
    if ( d->m_bIsLocalUrl && ::access( QFile::encodeName(d->m_url.path()), R_OK ) == -1 )
        return false;

    return true;
}

bool KFileItem::isWritable() const
{
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
    if ( d->m_bIsLocalUrl && ::access( QFile::encodeName(d->m_url.path()), W_OK ) == -1 )
        return false;

    return true;
}

bool KFileItem::isHidden() const
{
    // The kioslave can specify explicitly that a file is hidden or shown
    if ( d->m_hidden != KFileItemPrivate::Auto )
        return d->m_hidden == KFileItemPrivate::Hidden;

    // Prefer the filename that is part of the URL, in case the display name is different.
    QString fileName = d->m_url.fileName();
    if (fileName.isEmpty()) // e.g. "trash:/"
        fileName = d->m_strName;
    return fileName.length() > 1 && fileName[0] == '.';  // Just "." is current directory, not hidden.
}

bool KFileItem::isDir() const
{
    if ( d->m_fileMode == KFileItem::Unknown )
    {
        kDebug(7101) << "can't say -> false";
        return false; // can't say for sure, so no
    }
    return (S_ISDIR(d->m_fileMode));
/*
  if  (!S_ISDIR(d->m_fileMode)) {
  if (d->m_url.isLocalFile()) {
  KMimeType::Ptr ptr=KMimeType::findByUrl(d->m_url,0,true,true);
  if ((ptr!=0) && (ptr->is("directory/inode"))) return true;
  }
  return false
  } else return true;*/
}

bool KFileItem::isFile() const
{
    return !isDir();
}

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
    if ( QFileInfo(d->m_url.path()).isExecutable() )
        return true;

    return false;
}

QString KFileItem::getStatusBarInfo() const
{
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
        text += i18n ( " (Points to %1)", targetUrl().pathOrUrl());
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

QString KFileItem::getToolTipText(int maxcount) const
{
    // we can return QString() if no tool tip should be shown
    QString tip;
    KFileMetaInfo info = metaInfo();

    // the font tags are a workaround for the fact that the tool tip gets
    // screwed if the color scheme uses white as default text color
    const char* start = "<tr><td align=\"right\"><nobr><font color=\"black\">";
    const char* mid   = "&nbsp;</font></nobr></td><td><nobr><font color=\"black\">";
    const char* end   = "</font></nobr></td></tr>";

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
               QString("%1 (%2)").arg(KIO::convertSize(size()))
               .arg(KGlobal::locale()->formatNumber(size(), 0)) +
               end;

    tip += start + i18n("Modified:") + mid +
           timeString( KFileItem::ModificationTime ) + end
#ifndef Q_WS_WIN //TODO: show win32-specific permissions
           +start + i18n("Owner:") + mid + user() + " - " + group() + end +
           start + i18n("Permissions:") + mid +
           permissionsString() + end
#endif
           ;

    if (info.isValid())
    {
        tip += "<tr><td colspan=2><center><s>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</s></center></td></tr>";
        const QStringList keys = info.preferredKeys();

        // now the rest
        QStringList::ConstIterator it = keys.begin();
        for (int count = 0; count<maxcount && it!=keys.end() ; ++it)
        {
            KFileMetaInfoItem item = info.item( *it );
            if ( item.isValid() )
            {
                QString s = item.value().toString();
                if ( ( item.properties().attributes() & PredicateProperties::SqueezeText )
                     && s.length() > 50) {
                    s.truncate(47);
                    s.append("...");
                }
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

void KFileItem::run( QWidget* parentWidget ) const
{
    (void) new KRun( d->m_url, parentWidget, d->m_fileMode, d->m_bIsLocalUrl );
}

bool KFileItem::cmp( const KFileItem & item ) const
{
    return d->cmp(*item.d);
}

bool KFileItem::operator==(const KFileItem& other) const
{
    // is this enough?
    return d == other.d;
}

bool KFileItem::operator!=(const KFileItem& other) const
{
    return d != other.d;
}

void KFileItem::setUDSEntry( const KIO::UDSEntry& _entry, const KUrl& _url,
                             bool _delayedMimeTypes, bool _urlIsDirectory )
{
    d->m_entry = _entry;
    d->m_url = _url;
    d->m_strName.clear();
    d->m_strText.clear();
    d->m_iconName.clear();
    d->m_strLowerCaseName.clear();
    d->m_pMimeType = 0;
    d->m_fileMode = KFileItem::Unknown;
    d->m_permissions = KFileItem::Unknown;
    d->m_bMarked = false;
    d->m_bLink = false;
    d->m_bIsLocalUrl = _url.isLocalFile();
    d->m_bMimeTypeKnown = false;
    d->m_hidden = KFileItemPrivate::Auto;
    d->m_guessedMimeType.clear();
    d->m_metaInfo = KFileMetaInfo();
    d->m_delayedMimeTypes = _delayedMimeTypes;
    d->m_useIconNameCache = false;

    d->readUDSEntry( _urlIsDirectory );
    d->init();
}

KFileItem::operator QVariant() const
{
    return qVariantFromValue(*this);
}

void KFileItem::setExtraData( const void *key, void *value )
{
    if ( !key )
        return;

    d->m_extra.insert( key, value ); // replaces the value of key if already there
}

const void * KFileItem::extraData( const void *key ) const
{
    return d->m_extra.value( key, 0 );
}

void KFileItem::removeExtraData( const void *key )
{
    d->m_extra.remove( key );
}

QString KFileItem::permissionsString() const
{
    if (d->m_access.isNull() && d->m_permissions != KFileItem::Unknown)
        d->m_access = d->parsePermissions( d->m_permissions );

    return d->m_access;
}

// check if we need to cache this
QString KFileItem::timeString( FileTimes which ) const
{
    return KGlobal::locale()->formatDateTime( d->time(which) );
}

QString KFileItem::timeString( unsigned int which ) const
{
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

void KFileItem::setMetaInfo( const KFileMetaInfo & info ) const
{
    d->m_metaInfo = info;
}

KFileMetaInfo KFileItem::metaInfo(bool autoget, int) const
{
    if (autoget && !d->m_metaInfo.isValid())
    {
        bool isLocalUrl;
        KUrl url(mostLocalUrl(isLocalUrl));
        if (KGlobalSettings::showFilePreview(url))
            d->m_metaInfo = KFileMetaInfo(url);//, mimetype() );
    }
    return d->m_metaInfo;
}

void KFileItem::assign( const KFileItem & item )
{
    *this = item;
}

KUrl KFileItem::mostLocalUrl(bool &local) const
{
    QString local_path = localPath();

    if ( !local_path.isEmpty() )
    {
        local = true;
        KUrl url;
        url.setPath(local_path);
        return url;
    }
    else
    {
        local = d->m_bIsLocalUrl;
        return d->m_url;
    }
}

QDataStream & operator<< ( QDataStream & s, const KFileItem & a )
{
    // We don't need to save/restore anything that refresh() invalidates,
    // since that means we can re-determine those by ourselves.
    s << a.d->m_url;
    s << a.d->m_strName;
    s << a.d->m_strText;
    return s;
}

QDataStream & operator>> ( QDataStream & s, KFileItem & a )
{
    s >> a.d->m_url;
    s >> a.d->m_strName;
    s >> a.d->m_strText;
    a.d->m_bIsLocalUrl = a.d->m_url.isLocalFile();
    a.d->m_bMimeTypeKnown = false;
    a.refresh();
    return s;
}

KUrl KFileItem::url() const
{
    return d->m_url;
}

mode_t KFileItem::permissions() const
{
    return d->m_permissions;
}

mode_t KFileItem::mode() const
{
    return d->m_fileMode;
}

bool KFileItem::isLink() const
{
    return d->m_bLink;
}

bool KFileItem::isLocalFile() const
{
    return d->m_bIsLocalUrl;
}

QString KFileItem::text() const
{
    return d->m_strText;
}

QString KFileItem::name( bool lowerCase ) const
{
    if ( !lowerCase )
        return d->m_strName;
    else
        if ( d->m_strLowerCaseName.isNull() )
            d->m_strLowerCaseName = d->m_strName.toLower();
    return d->m_strLowerCaseName;
}

KUrl KFileItem::targetUrl() const
{
    const QString targetUrlStr = d->m_entry.stringValue( KIO::UDSEntry::UDS_TARGET_URL );
    if (!targetUrlStr.isEmpty())
      return KUrl(targetUrlStr);
    else
      return url();
}

/*
 * Mimetype handling.
 *
 * Initial state: m_pMimeType = 0.
 * When mimeTypePtr() is called first: fast mimetype determination,
 *   might either find an accurate mimetype (-> Final state), otherwise we
 *   set m_pMimeType but not m_bMimeTypeKnown (-> Intermediate state)
 * Intermediate state: determineMimeType() does the real determination -> Final state.
 *
 * If delayedMimeTypes isn't set, then we always go to the Final state directly.
 */

KMimeType::Ptr KFileItem::mimeTypePtr() const
{
    if (!d->m_pMimeType) {
        // On-demand fast (but not always accurate) mimetype determination
        Q_ASSERT(!d->m_url.isEmpty());
        bool isLocalUrl;
        KUrl url = mostLocalUrl(isLocalUrl);
        int accuracy;
        d->m_pMimeType = KMimeType::findByUrl( url, d->m_fileMode, isLocalUrl,
                                               // use fast mode if delayed mimetype determination can refine it later
                                               d->m_delayedMimeTypes, &accuracy );
        // If we used the "fast mode" (no sniffing), and we didn't get a perfect (extension-based) match,
        // then determineMimeType will be able to do better.
        const bool canDoBetter = d->m_delayedMimeTypes && accuracy < 100;
        //kDebug() << "finding mimetype for" << url() << ":" << m_pMimeType->name();
        d->m_bMimeTypeKnown = !canDoBetter;
    }
    return d->m_pMimeType;
}

KIO::UDSEntry KFileItem::entry() const
{
    return d->m_entry;
}

bool KFileItem::isMarked() const
{
    return d->m_bMarked;
}

void KFileItem::mark()
{
    d->m_bMarked = true;
}

void KFileItem::unmark()
{
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

KFileItem KFileItemList::findByUrl( const KUrl& url ) const {
    const_iterator it = begin();
    const const_iterator itend = end();
    for ( ; it != itend ; ++it ) {
        if ( (*it).url() == url ) {
            return *it;
        }
    }
    return KFileItem();
}

KUrl::List KFileItemList::urlList() const {
    KUrl::List lst;
    const_iterator it = begin();
    const const_iterator itend = end();
    for ( ; it != itend ; ++it ) {
        lst.append( (*it).url() );
    }
    return lst;
}

bool KFileItem::isDesktopFile() const
{
    // only local files
    bool isLocal;
    const KUrl url = mostLocalUrl(isLocal);
    if (!isLocal)
        return false;

    // only regular files
    if (!S_ISREG(d->m_fileMode))
        return false;

    // only if readable
    if (!isReadable())
        return false;

    // return true if desktop file
    return determineMimeType()->is("application/x-desktop");
}

