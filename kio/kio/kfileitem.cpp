// -*- c-basic-offset: 2 -*-
/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>
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

#include <sys/time.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>

#include <assert.h>
#include <unistd.h>

#include "kfileitem.h"

#include <qdatetime.h>
#include <qdir.h>
#include <qfile.h>
#include <qmap.h>
#include <QTextDocument>

#include <kdebug.h>
#include <kfilemetainfo.h>
#include <ksambashare.h>
#include <knfsshare.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kde_file.h>
#include <klocale.h>
#include <kmimetype.h>
#include <krun.h>

class KFileItem::KFileItemPrivate {
	public:
		QString iconName;
};

KFileItem::KFileItem( const KIO::UDSEntry& _entry, const KUrl& _url,
                      bool _determineMimeTypeOnDemand, bool _urlIsDirectory ) :
  m_entry( _entry ),
  m_url( _url ),
  m_pMimeType( 0 ),
  m_fileMode( KFileItem::Unknown ),
  m_permissions( KFileItem::Unknown ),
  m_bMarked( false ),
  m_bLink( false ),
  m_bIsLocalURL( _url.isLocalFile() ),
  m_bMimeTypeKnown( false ),
  m_hidden( Auto ),
  d(0)
{
  readUDSEntry( _urlIsDirectory );
  init( _determineMimeTypeOnDemand );
}

KFileItem::KFileItem( mode_t _mode, mode_t _permissions, const KUrl& _url, bool _determineMimeTypeOnDemand ) :
  m_entry(), // warning !
  m_url( _url ),
  m_strName( _url.fileName() ),
  m_strText( KIO::decodeFileName( m_strName ) ),
  m_pMimeType( 0 ),
  m_fileMode ( _mode ),
  m_permissions( _permissions ),
  m_bMarked( false ),
  m_bLink( false ),
  m_bIsLocalURL( _url.isLocalFile() ),
  m_bMimeTypeKnown( false ),
  m_hidden( Auto ),
  d(0)
{
  init( _determineMimeTypeOnDemand );
}

KFileItem::KFileItem( const KUrl &url, const QString &mimeType, mode_t mode )
:  m_url( url ),
  m_strName( url.fileName() ),
  m_strText( KIO::decodeFileName( m_strName ) ),
  m_pMimeType( 0 ),
  m_fileMode( mode ),
  m_permissions( KFileItem::Unknown ),
  m_bMarked( false ),
  m_bLink( false ),
  m_bIsLocalURL( url.isLocalFile() ),
  m_bMimeTypeKnown( !mimeType.isEmpty() ),
  m_hidden( Auto ),
  d(0)
{
  if (m_bMimeTypeKnown)
    m_pMimeType = KMimeType::mimeType( mimeType );

  init( false );
}

KFileItem::KFileItem( const KFileItem & item ) :
  d(0)
{
    assign( item );
}

KFileItem& KFileItem::operator=( const KFileItem & item )
{
    assign( item );
    return *this;
}

KFileItem::~KFileItem()
{
  delete d;
}

void KFileItem::init( bool _determineMimeTypeOnDemand )
{
  m_access.clear();
  m_size = (KIO::filesize_t) -1;
  //  metaInfo = KFileMetaInfo();
  for ( int i = 0; i < NumFlags; i++ )
      m_time[i] = (time_t) -1;

  // determine mode and/or permissions if unknown
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
      QByteArray path = QFile::encodeName(m_url.path( -1 ));
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
        m_time[ Modification ] = buf.st_mtime;
        m_time[ Access ] = buf.st_atime;
        if ( m_fileMode == KFileItem::Unknown )
          m_fileMode = mode & S_IFMT; // extract file type
        if ( m_permissions == KFileItem::Unknown )
          m_permissions = mode & 07777; // extract permissions
      }
    }
  }

  // determine the mimetype
  if (!m_pMimeType && !m_url.isEmpty())
  {
    bool accurate = false;
    bool isLocalURL;
    KUrl url = mostLocalURL(isLocalURL);

    m_pMimeType = KMimeType::findByURL( url, m_fileMode, isLocalURL,
                                        // use fast mode if not mimetype on demand
                                        _determineMimeTypeOnDemand, &accurate );
    //kDebug() << "finding mimetype for " << url.url() << " : " << m_pMimeType->name() << endl;
    // if we didn't use fast mode, or if we got a result, then this is the mimetype
    // otherwise, determineMimeType will be able to do better.
    m_bMimeTypeKnown = (!_determineMimeTypeOnDemand) || accurate;
  }
}

void KFileItem::readUDSEntry( bool _urlIsDirectory )
{
  // extract fields from the KIO::UDS Entry

  m_fileMode = m_entry.numberValue( KIO::UDS_FILE_TYPE );
  m_permissions = m_entry.numberValue( KIO::UDS_ACCESS );
  m_user = m_entry.stringValue( KIO::UDS_USER );
  m_group = m_entry.stringValue( KIO::UDS_GROUP );
  m_strName = m_entry.stringValue( KIO::UDS_NAME );
  m_strText = KIO::decodeFileName( m_strName );
  const QString urlStr = m_entry.stringValue( KIO::UDS_URL );
  bool UDS_URL_seen = !urlStr.isEmpty();
  if ( UDS_URL_seen ) {
      m_url = KUrl( urlStr );
      if ( m_url.isLocalFile() )
          m_bIsLocalURL = true;
  }
  const QString mimeTypeStr = m_entry.stringValue( KIO::UDS_MIME_TYPE );
  m_bMimeTypeKnown = !mimeTypeStr.isEmpty();
  if ( m_bMimeTypeKnown )
      m_pMimeType = KMimeType::mimeType( mimeTypeStr );

  m_guessedMimeType = m_entry.stringValue( KIO::UDS_GUESSED_MIME_TYPE );
  m_bLink = !m_entry.stringValue( KIO::UDS_LINK_DEST ).isEmpty(); // we don't store the link dest

  m_iconName = m_entry.stringValue( KIO::UDS_ICON_NAME );

  const int hiddenVal = m_entry.numberValue( KIO::UDS_HIDDEN, -1 );
  m_hidden = hiddenVal == 1 ? Hidden : ( hiddenVal == 0 ? Shown : Auto );

  // avoid creating these QStrings again and again
  static const QString& dot = KGlobal::staticQString(".");
  if ( _urlIsDirectory && !UDS_URL_seen && !m_strName.isEmpty() && m_strName != dot )
    m_url.addPath( m_strName );
}

void KFileItem::refresh()
{
  m_fileMode = KFileItem::Unknown;
  m_permissions = KFileItem::Unknown;
  m_pMimeType = 0L;
  m_user.clear();
  m_group.clear();
  m_metaInfo = KFileMetaInfo();
  m_hidden = Auto;

  // Basically, we can't trust any information we got while listing.
  // Everything could have changed...
  // Clearing m_entry makes it possible to detect changes in the size of the file,
  // the time information, etc.
  m_entry = KIO::UDSEntry();
  init( false );
}

void KFileItem::refreshMimeType()
{
  m_pMimeType = 0L;
  init( false ); // Will determine the mimetype
}

void KFileItem::setURL( const KUrl &url )
{
  m_url = url;
  setName( url.fileName() );
}

void KFileItem::setName( const QString& name )
{
  m_strName = name;
  m_strText = KIO::decodeFileName( m_strName );
}

QString KFileItem::linkDest() const
{
  // Extract it from the KIO::UDSEntry
  const QString linkStr = m_entry.stringValue( KIO::UDS_LINK_DEST );
  if ( !linkStr.isEmpty() )
    return linkStr;

  // If not in the KIO::UDSEntry, or if UDSEntry empty, use readlink() [if local URL]
  if ( m_bIsLocalURL )
  {
    char buf[1000];
    int n = readlink( QFile::encodeName(m_url.path( -1 )), buf, sizeof(buf)-1 );
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
  if ( m_bIsLocalURL )
  {
    return m_url.path();
  }

  // Extract the local path from the KIO::UDSEntry
  return m_entry.stringValue( KIO::UDS_LOCAL_PATH );
}

KIO::filesize_t KFileItem::size() const
{
  if ( m_size != (KIO::filesize_t) -1 )
    return m_size;

  // Extract it from the KIO::UDSEntry
  long long fieldVal = m_entry.numberValue( KIO::UDS_SIZE, -1 );
  if ( fieldVal != -1 ) {
    m_size = fieldVal;
    return m_size;
  }

  // If not in the KIO::UDSEntry, or if UDSEntry empty, use stat() [if local URL]
  if ( m_bIsLocalURL )
  {
    KDE_struct_stat buf;
    if ( KDE_stat( QFile::encodeName(m_url.path( -1 )), &buf ) == 0 )
        return buf.st_size;
  }
  return 0L;
}

bool KFileItem::hasExtendedACL() const
{
  // Check if the field exists; its value doesn't matter
  return m_entry.numberValue( KIO::UDS_EXTENDED_ACL, -1 ) != -1;
}

KACL KFileItem::ACL() const
{
  if ( hasExtendedACL() ) {
    // Extract it from the KIO::UDSEntry
    const QString fieldVal = m_entry.stringValue( KIO::UDS_ACL_STRING );
    if ( !fieldVal.isEmpty() )
      return KACL( fieldVal );
  }
  // create one from the basic permissions
  return KACL( m_permissions );
}

KACL KFileItem::defaultACL() const
{
  // Extract it from the KIO::UDSEntry
  const QString fieldVal = m_entry.stringValue( KIO::UDS_DEFAULT_ACL_STRING );
  if ( !fieldVal.isEmpty() )
    return KACL(fieldVal);
  else
    return KACL();
}


time_t KFileItem::time( unsigned int which ) const
{
  unsigned int mappedWhich = 0;

  switch( which ) {
    case KIO::UDS_MODIFICATION_TIME:
      mappedWhich = Modification;
      break;
    case KIO::UDS_ACCESS_TIME:
      mappedWhich = Access;
      break;
    case KIO::UDS_CREATION_TIME:
      mappedWhich = Creation;
      break;
  }

  if ( m_time[mappedWhich] != (time_t) -1 )
    return m_time[mappedWhich];

  // Extract it from the KIO::UDSEntry
  long long fieldVal = m_entry.numberValue( which, -1 );
  if ( fieldVal != -1 ) {
    m_time[mappedWhich] = static_cast<time_t>( fieldVal );
    return m_time[mappedWhich];
  }

  // If not in the KIO::UDSEntry, or if UDSEntry empty, use stat() [if local URL]
  if ( m_bIsLocalURL )
  {
    KDE_struct_stat buf;
    if ( KDE_stat( QFile::encodeName(m_url.path(-1)), &buf ) == 0 )
    {
        m_time[mappedWhich] = (which == KIO::UDS_MODIFICATION_TIME) ?
                               buf.st_mtime :
                               (which == KIO::UDS_ACCESS_TIME) ? buf.st_atime :
                               static_cast<time_t>(0); // We can't determine creation time for local files
        return m_time[mappedWhich];
    }
  }
  return static_cast<time_t>(0);
}


QString KFileItem::user() const
{
  if ( m_user.isEmpty() && m_bIsLocalURL )
  {
    KDE_struct_stat buff;
    if ( KDE_lstat( QFile::encodeName(m_url.path( -1 )), &buff ) == 0) // get uid/gid of the link, if it's a link
    {
      struct passwd *user = getpwuid( buff.st_uid );
      if ( user != 0L )
        m_user = QString::fromLocal8Bit(user->pw_name);
    }
  }
  return m_user;
}

QString KFileItem::group() const
{
#ifdef Q_OS_UNIX
  if (m_group.isEmpty() && m_bIsLocalURL )
  {
    KDE_struct_stat buff;
    if ( KDE_lstat( QFile::encodeName(m_url.path( -1 )), &buff ) == 0) // get uid/gid of the link, if it's a link
    {
      struct group *ge = getgrgid( buff.st_gid );
      if ( ge != 0L ) {
        m_group = QString::fromLocal8Bit(ge->gr_name);
        if (m_group.isEmpty())
          m_group.sprintf("%d",ge->gr_gid);
      } else
        m_group.sprintf("%d",buff.st_gid);
    }
  }
#endif
  return m_group;
}

QString KFileItem::mimetype() const
{
  KFileItem * that = const_cast<KFileItem *>(this);
  return that->determineMimeType()->name();
}

KMimeType::Ptr KFileItem::determineMimeType()
{
  if ( !m_pMimeType || !m_bMimeTypeKnown )
  {
    bool isLocalURL;
    KUrl url = mostLocalURL(isLocalURL);

    m_pMimeType = KMimeType::findByURL( url, m_fileMode, isLocalURL );
    //kDebug() << "finding mimetype for " << url.url() << " : " << m_pMimeType->name() << endl;
    m_bMimeTypeKnown = true;
  }

  return m_pMimeType;
}

bool KFileItem::isMimeTypeKnown() const
{
  // The mimetype isn't known if determineMimeType was never called (on-demand determination)
  // or if this fileitem has a guessed mimetype (e.g. ftp symlink) - in which case
  // it always remains "not fully determined"
  return m_bMimeTypeKnown && m_guessedMimeType.isEmpty();
}

QString KFileItem::mimeComment()
{
 KMimeType::Ptr mType = determineMimeType();

 bool isLocalURL;
 KUrl url = mostLocalURL(isLocalURL);

 QString comment = mType->comment( url, isLocalURL );
 //kDebug() << "finding comment for " << url.url() << " : " << m_pMimeType->name() << endl;
  if (!comment.isEmpty())
    return comment;
  else
    return mType->name();
}

QString KFileItem::iconName()
{
  if (d && (!d->iconName.isEmpty())) return d->iconName;

  bool isLocalURL;
  KUrl url = mostLocalURL(isLocalURL);

  //kDebug() << "finding icon for " << url.url() << " : " << m_pMimeType->name() << endl;
  return determineMimeType()->icon(url, isLocalURL);
}

int KFileItem::overlays() const
{
  int _state = 0;
  if ( m_bLink )
      _state |= KIcon::LinkOverlay;

  if ( !S_ISDIR( m_fileMode ) // Locked dirs have a special icon, use the overlay for files only
       && !isReadable())
     _state |= KIcon::LockOverlay;

  if ( isHidden() )
     _state |= KIcon::HiddenOverlay;

  if( S_ISDIR( m_fileMode ) && m_bIsLocalURL)
  {
    if (KSambaShare::instance()->isDirectoryShared( m_url.path() ) ||
        KNFSShare::instance()->isDirectoryShared( m_url.path() ))
    {
      //kDebug()<<"KFileShare::isDirectoryShared : "<<m_url.path()<<endl;
      _state |= KIcon::ShareOverlay;
    }
  }

  if ( m_pMimeType->name() == "application/x-gzip" && m_url.fileName().endsWith( QLatin1String( ".gz" ) ) )
     _state |= KIcon::ZipOverlay;
  return _state;
}

QPixmap KFileItem::pixmap( int _size, int _state ) const
{
  if (d && (!d->iconName.isEmpty()))
     return DesktopIcon(d->iconName,_size,_state);

  if ( !m_pMimeType )
  {
    static const QString & defaultFolderIcon =
       KGlobal::staticQString(KMimeType::mimeType( "inode/directory" )->KServiceType::icon());

    if ( S_ISDIR( m_fileMode ) )
     return DesktopIcon( defaultFolderIcon, _size, _state );

    return DesktopIcon( "unknown", _size, _state );
  }

  _state |= overlays();

  KMimeType::Ptr mime;
  // Use guessed mimetype if the main one hasn't been determined for sure
  if ( !m_bMimeTypeKnown && !m_guessedMimeType.isEmpty() )
      mime = KMimeType::mimeType( m_guessedMimeType );
  else
      mime = m_pMimeType;

  // Support for gzipped files: extract mimetype of contained file
  // See also the relevant code in overlays, which adds the zip overlay.
  if ( mime->name() == "application/x-gzip" && m_url.fileName().endsWith( QLatin1String( ".gz" ) ) )
  {
      KUrl sf;
      sf.setPath( m_url.path().left( m_url.path().length() - 3 ) );
      //kDebug() << "KFileItem::pixmap subFileName=" << subFileName << endl;
      mime = KMimeType::findByURL( sf, 0, m_bIsLocalURL );
  }

  bool isLocalURL;
  KUrl url = mostLocalURL(isLocalURL);

  QPixmap p = mime->pixmap( url, KIcon::Desktop, _size, _state );
  //kDebug() << "finding pixmap for " << url.url() << " : " << mime->name() << endl;
  if (p.isNull())
      kWarning() << "Pixmap not found for mimetype " << m_pMimeType->name() << endl;

  return p;
}

bool KFileItem::isReadable() const
{
  /*
  struct passwd * user = getpwuid( geteuid() );
  bool isMyFile = (QString::fromLocal8Bit(user->pw_name) == m_user);
  // This gets ugly for the group....
  // Maybe we want a static QString for the user and a static QStringList
  // for the groups... then we need to handle the deletion properly...
  */

  // No read permission at all
  if ( !(S_IRUSR & m_permissions) && !(S_IRGRP & m_permissions) && !(S_IROTH & m_permissions) )
      return false;

  // Or if we can't read it [using ::access()] - not network transparent
  else if ( m_bIsLocalURL && ::access( QFile::encodeName(m_url.path()), R_OK ) == -1 )
      return false;

  return true;
}

bool KFileItem::isWritable() const
{
  /*
  struct passwd * user = getpwuid( geteuid() );
  bool isMyFile = (QString::fromLocal8Bit(user->pw_name) == m_user);
  // This gets ugly for the group....
  // Maybe we want a static QString for the user and a static QStringList
  // for the groups... then we need to handle the deletion properly...
  */

  // No write permission at all
  if ( !(S_IWUSR & m_permissions) && !(S_IWGRP & m_permissions) && !(S_IWOTH & m_permissions) )
      return false;

  // Or if we can't read it [using ::access()] - not network transparent
  else if ( m_bIsLocalURL && ::access( QFile::encodeName(m_url.path()), W_OK ) == -1 )
      return false;

  return true;
}

bool KFileItem::isHidden() const
{
  if ( m_hidden != Auto )
      return m_hidden == Hidden;

  if ( !m_url.isEmpty() )
      return m_url.fileName()[0] == '.';
  else // should never happen
      return m_strName[0] == '.';
}

bool KFileItem::isDir() const
{
  if ( m_fileMode == KFileItem::Unknown )
  {
    kDebug() << " KFileItem::isDir can't say -> false " << endl;
    return false; // can't say for sure, so no
  }
  return (S_ISDIR(m_fileMode));
/*
  if  (!S_ISDIR(m_fileMode)) {
	if (m_url.isLocalFile()) {
		KMimeType::Ptr ptr=KMimeType::findByURL(m_url,0,true,true);
		if ((ptr!=0) && (ptr->is("directory/inode"))) return true;
	}
	return false
  } else return true;*/
}

bool KFileItem::acceptsDrops()
{
  // A directory ?
  if ( S_ISDIR( mode() ) ) {
      return isWritable();
  }

  // But only local .desktop files and executables
  if ( !m_bIsLocalURL )
    return false;

  if ( mimetype() == "application/x-desktop")
    return true;

  // Executable, shell script ... ?
  if ( ::access( QFile::encodeName(m_url.path()), X_OK ) == 0 )
    return true;

  return false;
}

QString KFileItem::getStatusBarInfo()
{
  QString text = m_strText;

  if ( m_bLink )
  {
      QString comment = determineMimeType()->comment( m_url, m_bIsLocalURL );
      QString tmp;
      if ( comment.isEmpty() )
        tmp = i18n ( "Symbolic Link" );
      else
        tmp = i18n("%1 (Link)").arg(comment);
      text += "->";
      text += linkDest();
      text += "  ";
      text += tmp;
  }
  else if ( S_ISREG( m_fileMode ) )
  {
      text += QString(" (%1)").arg( KIO::convertSize( size() ) );
      text += "  ";
      text += mimeComment();
  }
  else if ( S_ISDIR ( m_fileMode ) )
  {
      text += "/  ";
      text += mimeComment();
  }
  else
  {
      text += "  ";
      text += mimeComment();
  }
  return text;
}

QString KFileItem::getToolTipText(int maxcount)
{
  // we can return QString() if no tool tip should be shown
  QString tip;
  KFileMetaInfo info = metaInfo();

  // the font tags are a workaround for the fact that the tool tip gets
  // screwed if the color scheme uses white as default text color
  const char* start = "<tr><td><nobr><font color=\"black\">";
  const char* mid   = "</font></nobr></td><td><nobr><font color=\"black\">";
  const char* end   = "</font></nobr></td></tr>";

  tip = "<table cellspacing=0 cellpadding=0>";

  tip += start + i18n("Name:") + mid + text() + end;
  tip += start + i18n("Type:") + mid;

  QString type = Qt::escape(mimeComment());
  if ( m_bLink ) {
   tip += i18n("Link to %1 (%2)").arg(linkDest(), type) + end;
  } else
    tip += type + end;

  if ( !S_ISDIR ( m_fileMode ) )
    tip += start + i18n("Size:") + mid +
           QString("%1 (%2)").arg(KIO::convertSize(size()))
                             .arg(KGlobal::locale()->formatNumber(size(), 0)) +
           end;

  tip += start + i18n("Modified:") + mid +
         timeString( KIO::UDS_MODIFICATION_TIME) + end
#ifndef Q_WS_WIN //TODO: show win32-specific permissions
         +start + i18n("Owner:") + mid + user() + " - " + group() + end +
         start + i18n("Permissions:") + mid +
         parsePermissions(m_permissions) + end
#endif
				 ;

  if (info.isValid() && !info.isEmpty() )
  {
    tip += "<tr><td colspan=2><center><s>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</s></center></td></tr>";
    QStringList keys = info.preferredKeys();

    // now the rest
    QStringList::Iterator it = keys.begin();
    for (int count = 0; count<maxcount && it!=keys.end() ; ++it)
    {
      KFileMetaInfoItem item = info.item( *it );
      if ( item.isValid() )
      {
        QString s = item.string();
        if ( ( item.attributes() & KFileMimeTypeInfo::SqueezeText )
             && s.length() > 50) {
            s.truncate(47);
            s.append("...");
        }
        if ( !s.isEmpty() )
        {
          count++;
          tip += start +
                   Qt::escape( item.translatedKey() ) + ":" +
                 mid +
                   Qt::escape( s ) +
                 end;
        }

      }
    }
  }
  tip += "</table>";

  //kDebug() << "making this the tool tip rich text:\n";
  //kDebug() << tip << endl;

  return tip;
}

void KFileItem::run()
{
  KUrl url = m_url;
  // When clicking on a link to e.g. $HOME from the desktop, we want to open $HOME
  // But when following a link on the FTP site, the target be an absolute path
  // that doesn't work in the URL. So we resolve links only on the local filesystem.
  if ( m_bLink && m_bIsLocalURL )
    url = KUrl( m_url, linkDest() );

  // It might be faster to pass skip that when we know the mimetype,
  // and just call KRun::runURL. But then we need to use mostLocalURL()
  // for application/x-desktop files, to be able to execute them.
  (void) new KRun( url, (QWidget*)0, m_fileMode, m_bIsLocalURL );
}

bool KFileItem::cmp( const KFileItem & item )
{
    return ( m_strName == item.m_strName
             && m_bIsLocalURL == item.m_bIsLocalURL
             && m_fileMode == item.m_fileMode
             && m_permissions == item.m_permissions
             && m_user == item.m_user
             && m_group == item.m_group
             && m_bLink == item.m_bLink
             && m_hidden == item.m_hidden
             && size() == item.size()
             && time(KIO::UDS_MODIFICATION_TIME) == item.time(KIO::UDS_MODIFICATION_TIME)
             && (!d || !item.d || d->iconName == item.d->iconName) );

    // Don't compare the mimetypes here. They might not be known, and we don't want to
    // do the slow operation of determining them here.
}

void KFileItem::assign( const KFileItem & item )
{
    if ( this == &item )
        return;
    m_entry = item.m_entry;
    m_url = item.m_url;
    m_bIsLocalURL = item.m_bIsLocalURL;
    m_strName = item.m_strName;
    m_strText = item.m_strText;
    m_fileMode = item.m_fileMode;
    m_permissions = item.m_permissions;
    m_user = item.m_user;
    m_group = item.m_group;
    m_bLink = item.m_bLink;
    m_pMimeType = item.m_pMimeType;
    m_strLowerCaseName = item.m_strLowerCaseName;
    m_bMimeTypeKnown = item.m_bMimeTypeKnown;
    m_hidden = item.m_hidden;
    m_guessedMimeType   = item.m_guessedMimeType;
    m_access            = item.m_access;
    m_metaInfo          = item.m_metaInfo;
    for ( int i = 0; i < NumFlags; i++ )
        m_time[i] = item.m_time[i];
    m_size = item.m_size;
    // note: m_extra is NOT copied, as we'd have no control over who is
    // deleting the data or not.

    // We had a mimetype previously (probably), so we need to re-determine it
    determineMimeType();

    if ( item.d ) {
        if ( !d )
            d = new KFileItemPrivate;
        d->iconName = item.d->iconName;
    } else {
        delete d;
        d = 0;
    }
}

void KFileItem::setUDSEntry( const KIO::UDSEntry& _entry, const KUrl& _url,
    bool _determineMimeTypeOnDemand, bool _urlIsDirectory )
{
  m_entry = _entry;
  m_url = _url;
  m_strName.clear();
  m_strText.clear();
  m_user.clear();
  m_group.clear();
  m_strLowerCaseName.clear();
  m_pMimeType = 0;
  m_fileMode = KFileItem::Unknown;
  m_permissions = KFileItem::Unknown;
  m_bMarked = false;
  m_bLink = false;
  m_bIsLocalURL = _url.isLocalFile();
  m_bMimeTypeKnown = false;
  m_hidden = Auto;
  m_guessedMimeType.clear();
  m_metaInfo = KFileMetaInfo();

  if ( d )
    d->iconName.clear();

  readUDSEntry( _urlIsDirectory );
  init( _determineMimeTypeOnDemand );
}

void KFileItem::setExtraData( const void *key, void *value )
{
    if ( !key )
        return;

    m_extra.replace( key, value );
}

const void * KFileItem::extraData( const void *key ) const
{
    QMap<const void*,void*>::const_iterator it = m_extra.find( key );
    if ( it != m_extra.end() )
        return it.value();
    return 0L;
}

void * KFileItem::extraData( const void *key )
{
    QMap<const void*,void*>::iterator it = m_extra.find( key );
    if ( it != m_extra.end() )
        return it.value();
    return 0L;
}

void KFileItem::removeExtraData( const void *key )
{
    m_extra.remove( key );
}

QString KFileItem::permissionsString() const
{
    if (m_access.isNull())
      m_access = parsePermissions( m_permissions );

    return m_access;
}

QString KFileItem::parsePermissions(mode_t perm) const
{
    char p[] = "---------- ";

    if (isDir())
	p[0]='d';
    else if (isLink())
	p[0]='l';

    if (perm & QFile::ReadUser)
	p[1]='r';
    if (perm & QFile::WriteUser)
	p[2]='w';
    if ((perm & QFile::ExeUser) && !(perm & S_ISUID)) p[3]='x';
    else if ((perm & QFile::ExeUser) && (perm & S_ISUID)) p[3]='s';
    else if (!(perm & QFile::ExeUser) && (perm & S_ISUID)) p[3]='S';

    if (perm & QFile::ReadGroup)
	p[4]='r';
    if (perm & QFile::WriteGroup)
	p[5]='w';
    if ((perm & QFile::ExeGroup) && !(perm & S_ISGID)) p[6]='x';
    else if ((perm & QFile::ExeGroup) && (perm & S_ISGID)) p[6]='s';
    else if (!(perm & QFile::ExeGroup) && (perm & S_ISGID)) p[6]='S';

    if (perm & QFile::ReadOther)
	p[7]='r';
    if (perm & QFile::WriteOther)
	p[8]='w';
    if ((perm & QFile::ExeOther) && !(perm & S_ISVTX)) p[9]='x';
    else if ((perm & QFile::ExeOther) && (perm & S_ISVTX)) p[9]='t';
    else if (!(perm & QFile::ExeOther) && (perm & S_ISVTX)) p[9]='T';

    if (hasExtendedACL())
        p[10]='+';

    return QLatin1String(p);
}

// check if we need to cache this
QString KFileItem::timeString( unsigned int which ) const
{
    QDateTime t;
    t.setTime_t( time(which) );
    return KGlobal::locale()->formatDateTime( t );
}

void KFileItem::setMetaInfo( const KFileMetaInfo & info )
{
    m_metaInfo = info;
}

const KFileMetaInfo & KFileItem::metaInfo(bool autoget, int) const
{
    bool isLocalURL;
    KUrl url = mostLocalURL(isLocalURL);

    if ( autoget && !m_metaInfo.isValid() &&
         KGlobalSettings::showFilePreview(url) )
    {
        m_metaInfo = KFileMetaInfo( url, mimetype() );
    }

    return m_metaInfo;
}

KUrl KFileItem::mostLocalURL(bool &local) const
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
        local = m_bIsLocalURL;
        return m_url;
    }
}

void KFileItem::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

QDataStream & operator<< ( QDataStream & s, const KFileItem & a )
{
    // We don't need to save/restore anything that refresh() invalidates,
    // since that means we can re-determine those by ourselves.
    s << a.m_url;
    s << a.m_strName;
    s << a.m_strText;
    return s;
}

QDataStream & operator>> ( QDataStream & s, KFileItem & a )
{
    s >> a.m_url;
    s >> a.m_strName;
    s >> a.m_strText;
    a.m_bIsLocalURL = a.m_url.isLocalFile();
    a.m_bMimeTypeKnown = false;
    a.refresh();
    return s;
}
