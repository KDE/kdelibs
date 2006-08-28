/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *                     David Faure   <faure@kde.org>
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

#include <config.h>

#include "kmimetype.h"
#include "kmimetypefactory.h"
#include "kmimemagic.h"
#include "kprotocolmanager.h"
#include <kde_file.h>

#include <kmessageboxwrapper.h>

#include <kdebug.h>
#include <kdesktopfile.h>
#include <kiconloader.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <ksycoca.h>

#include <qset.h>
#include <qstring.h>
#include <qfile.h>
#include <QtDBus/QtDBus>

#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include "kmimetypefactory.h"

template class KSharedPtr<KMimeType>;

KMimeType::Ptr KMimeType::s_pDefaultType;
bool KMimeType::s_bChecked = false;

void KMimeType::buildDefaultType()
{
  assert ( !s_pDefaultType );
  // Try to find the default type
  KMimeType::Ptr mime = KMimeTypeFactory::self()->
        findMimeTypeByName( defaultMimeType() );

  if (mime)
  {
      s_pDefaultType = mime;
  }
  else
  {
     errorMissingMimeType( defaultMimeType() );
     KStandardDirs stdDirs;
     QString sDefaultMimeType = stdDirs.resourceDirs("mime").first()+defaultMimeType()+".desktop";
     s_pDefaultType = new KMimeType( sDefaultMimeType, defaultMimeType(),
                                     "unknown", "mime", QStringList() );
  }
}

KMimeType::Ptr KMimeType::defaultMimeTypePtr()
{
  if ( !s_pDefaultType ) // we need a default type first
    buildDefaultType();
  return s_pDefaultType;
}

// Check for essential mimetypes
void KMimeType::checkEssentialMimeTypes()
{
  if ( s_bChecked ) // already done
    return;
  if ( !s_pDefaultType ) // we need a default type first
    buildDefaultType();

  s_bChecked = true; // must be done before building mimetypes

  // No Mime-Types installed ?
  // Lets do some rescue here.
  if ( !KMimeTypeFactory::self()->checkMimeTypes() )
  {
    KMessageBoxWrapper::error( 0L, i18n( "No mime types installed." ) );
    return; // no point in going any further
  }

  if ( !KMimeType::mimeType( "inode/directory" ) )
    errorMissingMimeType( "inode/directory" );
  if ( !KMimeType::mimeType( "inode/directory-locked" ) )
    errorMissingMimeType( "inode/directory-locked" );
  if ( !KMimeType::mimeType( "inode/blockdevice" ) )
    errorMissingMimeType( "inode/blockdevice" );
  if ( !KMimeType::mimeType( "inode/chardevice" ) )
    errorMissingMimeType( "inode/chardevice" );
  if ( !KMimeType::mimeType( "inode/socket" ) )
    errorMissingMimeType( "inode/socket" );
  if ( !KMimeType::mimeType( "inode/fifo" ) )
    errorMissingMimeType( "inode/fifo" );
  if ( !KMimeType::mimeType( "application/x-shellscript" ) )
    errorMissingMimeType( "application/x-shellscript" );
  if ( !KMimeType::mimeType( "application/x-executable" ) )
    errorMissingMimeType( "application/x-executable" );
  if ( !KMimeType::mimeType( "application/x-desktop" ) )
    errorMissingMimeType( "application/x-desktop" );
}

void KMimeType::errorMissingMimeType( const QString& _type )
{
  QString tmp = i18n( "Could not find mime type\n%1" ,  _type );

  KMessageBoxWrapper::sorry( 0, tmp );
}

KMimeType::Ptr KMimeType::mimeType( const QString& _name )
{
  KMimeType::Ptr mime = KMimeTypeFactory::self()->findMimeTypeByName( _name );
#if 0
  // was in kde3, but is inconsistent with KServiceType::serviceType,
  // and it breaks code that looks/creates offers, since application/octet-stream
  // is used instead.
  if ( !mime || !mime->isType( KST_KMimeType ) )
  {
    if ( !s_pDefaultType )
      buildDefaultType();
    return s_pDefaultType;
  }

  // We got a mimetype
#else
  // findMimeTypeByName() may return NULL if there exist no such mimetype. If
  // that's the case we need to return the default mimetype.
  if ( !mime ) {
    mime = defaultMimeTypePtr();
    Q_ASSERT(mime);
  }
#endif

  return mime;
}

KMimeType::List KMimeType::allMimeTypes()
{
  return KMimeTypeFactory::self()->allMimeTypes();
}

KMimeType::Ptr KMimeType::findByUrl( const KUrl& _url, mode_t _mode,
                                     bool _is_local_file, bool _fast_mode )
{
  checkEssentialMimeTypes();
  QString path = _url.path();

  if ( !_fast_mode && !_is_local_file && _url.isLocalFile() )
    _is_local_file = true;

  if ( !_fast_mode && _is_local_file && (_mode == 0 || _mode == (mode_t)-1) )
  {
    KDE_struct_stat buff;
    if ( KDE_stat( QFile::encodeName(path), &buff ) != -1 )
      _mode = buff.st_mode;
  }

  // Look at mode_t first
  if ( S_ISDIR( _mode ) )
  {
    // Special hack for local files. We want to see whether we
    // are allowed to enter the directory
    if ( _is_local_file )
    {
      if ( access( QFile::encodeName(path), R_OK ) == -1 )
        return mimeType( "inode/directory-locked" );
    }
    return mimeType( "inode/directory" );
  }
  if ( S_ISCHR( _mode ) )
    return mimeType( "inode/chardevice" );
  if ( S_ISBLK( _mode ) )
    return mimeType( "inode/blockdevice" );
  if ( S_ISFIFO( _mode ) )
    return mimeType( "inode/fifo" );
  if ( S_ISSOCK( _mode ) )
    return mimeType( "inode/socket" );
  // KMimeMagic can do that better for local files
  if ( !_is_local_file && S_ISREG( _mode ) && ( _mode & ( S_IXUSR | S_IXGRP | S_IXOTH ) ) )
    return mimeType( "application/x-executable" );

  QString fileName ( _url.fileName() );

  static const QString& slash = KGlobal::staticQString("/");
  if ( ! fileName.isNull() && !path.endsWith( slash ) )
  {
      // Try to find it out by looking at the filename
      KMimeType::Ptr mime( KMimeTypeFactory::self()->findFromPattern( fileName ) );
      if ( mime )
      {
        // Found something - can we trust it ? (e.g. don't trust *.pl over HTTP, could be anything)
        if ( _is_local_file || _url.hasSubUrl() || // Explicitly trust suburls
             KProtocolInfo::determineMimetypeFromExtension( _url.protocol() ) )
        {
            if ( _is_local_file && !_fast_mode ) {
                if ( mime->patternsAccuracy()<100 )
                {
                    KMimeMagicResult* result =
                            KMimeMagic::self()->findFileType( path );

                    if ( result && result->isValid() )
                        return mimeType( result->mimeType() );
                }
            }

            return mime;
        }
      }

      static const QString& dotdesktop = KGlobal::staticQString(".desktop");
      static const QString& dotkdelnk = KGlobal::staticQString(".kdelnk");
      static const QString& dotdirectory = KGlobal::staticQString(".directory");

      // Another filename binding, hardcoded, is .desktop:
      if ( fileName.endsWith( dotdesktop ) )
        return mimeType( "application/x-desktop" );
      // Another filename binding, hardcoded, is .kdelnk;
      // this is preserved for backwards compatibility
      if ( fileName.endsWith( dotkdelnk ) )
        return mimeType( "application/x-desktop" );
      // .directory files are detected as x-desktop by mimemagic
      // but don't have a Type= entry. Better cheat and say they are text files
      if ( fileName == dotdirectory )
        return mimeType( "text/plain" );
    }

  if ( !_is_local_file || _fast_mode )
  {
    QString def = KProtocolManager::defaultMimetype( _url );
    if ( !def.isEmpty() && def != defaultMimeType() )
    {
       // The protocol says it always returns a given mimetype (e.g. text/html for "man:")
       return mimeType( def );
    }
    if ( path.endsWith( slash ) || path.isEmpty() )
    {
      // We have no filename at all. Maybe the protocol has a setting for
      // which mimetype this means (e.g. directory).
      // For HTTP (def==defaultMimeType()) we don't assume anything,
      // because of redirections (e.g. freshmeat downloads).
      if ( def.isEmpty() )
      {
          // Assume inode/directory, if the protocol supports listing.
          if ( KProtocolManager::supportsListing( _url ) )
              return mimeType( QLatin1String("inode/directory") );
          else
              return defaultMimeTypePtr(); // == 'no idea', e.g. for "data:,foo/"
      }
    }

    // No more chances for non local URLs
    return defaultMimeTypePtr();
  }

  // Do some magic for local files
  //kDebug(7009) << QString("Mime Type finding for '%1'").arg(path) << endl;
  KMimeMagicResult* result = KMimeMagic::self()->findFileType( path );

  // If we still did not find it, we must assume the default mime type
  if ( !result || !result->isValid() )
    return defaultMimeTypePtr();

  // The mimemagic stuff was successful
  return mimeType( result->mimeType() );
}

KMimeType::Ptr KMimeType::findByUrl( const KUrl& _url, mode_t _mode,
                                     bool _is_local_file, bool _fast_mode,
                                     bool *accurate )
{
    KMimeType::Ptr mime = findByUrl(_url, _mode, _is_local_file, _fast_mode);
    if (accurate) *accurate = !(_fast_mode) || ((mime->patternsAccuracy() == 100) && mime != defaultMimeTypePtr());
    return mime;
}

QString KMimeType::extractKnownExtension(const QString &fileName)
{
    QString pattern;
    KMimeTypeFactory::self()->findFromPattern( fileName, &pattern );
    if ( !pattern.isEmpty() && pattern.startsWith( "*." ) && pattern.indexOf('*', 2) == -1 )
        return pattern.mid( 2 ); // remove the leading "*."
    return QString();
}

KMimeType::Ptr KMimeType::findByPath( const QString& path, mode_t mode, bool fast_mode )
{
    KUrl u;
    u.setPath(path);
    return findByUrl( u, mode, true, fast_mode );
}

KMimeType::Ptr KMimeType::findByContent( const QByteArray &data, int *accuracy )
{
  KMimeMagicResult *result = KMimeMagic::self()->findBufferType(data);
  if (accuracy)
      *accuracy = result->accuracy();
  return mimeType( result->mimeType() );
}

KMimeType::Ptr KMimeType::findByFileContent( const QString &fileName, int *accuracy )
{
  KMimeMagicResult *result = KMimeMagic::self()->findFileType(fileName);
  if (accuracy)
      *accuracy = result->accuracy();
  return mimeType( result->mimeType() );
}

#define GZIP_MAGIC1	0x1f
#define GZIP_MAGIC2	0x8b

KMimeType::Format KMimeType::findFormatByFileContent( const QString &fileName )
{
  KMimeType::Format result;
  result.compression = Format::NoCompression;
  KMimeType::Ptr mime = findByPath(fileName);
  if (mime->name() == defaultMimeType())
     mime = findByFileContent(fileName);

  result.text = mime->name().startsWith("text/");
  QVariant v = mime->property("X-KDE-text");
  if (v.isValid())
     result.text = v.toBool();

  if (mime->name().startsWith("inode/"))
     return result;

  QFile f(fileName);
  if (f.open(QIODevice::ReadOnly))
  {
     const QByteArray buf = f.read(3);
     if ((buf.size() >= 2) && ((unsigned char)buf[0] == GZIP_MAGIC1) && ((unsigned char)buf[1] == GZIP_MAGIC2))
        result.compression = Format::GZipCompression;
  }
  return result;
}

// Used to create the default mimetype, in particular
KMimeType::KMimeType( const QString & _fullpath, const QString& _type, const QString& _icon,
                      const QString& _comment, const QStringList& _patterns )
  : KServiceType( _fullpath, _type, _comment ), m_strIcon( _icon ), d( 0 )
{
  m_lstPatterns = _patterns;
}

KMimeType::KMimeType( const QString & _fullpath ) : KServiceType( _fullpath ), d( 0 )
{
  KDesktopFile _cfg( _fullpath, true );
  init ( &_cfg );

  if ( !isValid() )
    kWarning(7009) << "mimetype not valid '" << name() << "' (missing entry in the file ?)" << endl;
}

KMimeType::KMimeType( KDesktopFile *config ) : KServiceType( config ), d( 0 )
{
  init( config );

  if ( !isValid() )
    kWarning(7009) << "mimetype not valid '" << name() << "' (missing entry in the file ?)" << endl;
}

void KMimeType::init( KDesktopFile * config )
{
  config->setDesktopGroup();
  m_strIcon = config->readIcon();
  m_lstPatterns = config->readEntry( "Patterns", QStringList(), ';' );

  // Read the X-KDE-AutoEmbed setting and store it in the properties map
  QString XKDEAutoEmbed = QLatin1String("X-KDE-AutoEmbed");
  if ( config->hasKey( XKDEAutoEmbed ) )
    m_mapProps.insert( XKDEAutoEmbed, config->readEntry( XKDEAutoEmbed, false ) );

  QString XKDEText = QLatin1String("X-KDE-text");
  if ( config->hasKey( XKDEText ) )
    m_mapProps.insert( XKDEText, config->readEntry( XKDEText, false ) );

  QString XKDEIsAlso = QLatin1String("X-KDE-IsAlso");
  if ( config->hasKey( XKDEIsAlso ) ) {
    QString inherits = config->readEntry( XKDEIsAlso, QString() );
    if ( inherits != name() )
        m_mapProps.insert( XKDEIsAlso, inherits );
    else
        kWarning(7009) << "Error: " << inherits << " inherits from itself!!!!" << endl;
  }

  QString XKDEPatternsAccuracy = QLatin1String("X-KDE-PatternsAccuracy");
  if ( config->hasKey( XKDEPatternsAccuracy ) )
    m_mapProps.insert( XKDEPatternsAccuracy, config->readEntry( XKDEPatternsAccuracy, QString() ) );
}

KMimeType::KMimeType( QDataStream& _str, int offset ) : KServiceType( _str, offset ), d( 0 )
{
  loadInternal( _str ); // load our specific stuff
}

void KMimeType::load( QDataStream& _str )
{
  KServiceType::load( _str );
  loadInternal( _str );
}

void KMimeType::loadInternal( QDataStream& _str )
{
  // kDebug(7009) << "KMimeType::load( QDataStream& ) : loading list of patterns" << endl;
  _str >> m_lstPatterns >> m_strIcon;
}

void KMimeType::save( QDataStream& _str )
{
  KServiceType::save( _str );
  // Warning adding/removing fields here involves a binary incompatible change - update version
  // number in ksycoca.h
  _str << m_lstPatterns << m_strIcon;
}

QVariant KMimeType::property( const QString& _name ) const
{
  if ( _name == "Patterns" )
    return QVariant( m_lstPatterns );
  if ( _name == "Icon" )
    return QVariant( m_strIcon );

  return KServiceType::property( _name );
}

QStringList KMimeType::propertyNames() const
{
  QStringList res = KServiceType::propertyNames();
  res.append( "Patterns" );
  res.append( "Icon" );

  return res;
}

KMimeType::~KMimeType()
{
}

QPixmap KMimeType::pixmap( K3Icon::Group _group, int _force_size, int _state,
                           QString * _path ) const
{
  return KGlobal::iconLoader()->loadMimeTypeIcon( iconName(), _group, _force_size, _state, _path );
}

QPixmap KMimeType::pixmap( const KUrl& _url, K3Icon::Group _group, int _force_size,
                           int _state, QString * _path ) const
{
  return KGlobal::iconLoader()->loadMimeTypeIcon( iconName( _url ), _group, _force_size, _state, _path );
}

QString KMimeType::iconForUrl( const KUrl & _url, mode_t _mode )
{
  return iconNameForUrl(_url,_mode);
}

QString KMimeType::iconNameForUrl( const KUrl & _url, mode_t _mode )
{
    const KMimeType::Ptr mt = findByUrl( _url, _mode, _url.isLocalFile(),
                                         false /*HACK*/);
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

QString KMimeType::parentMimeType() const
{
  QVariant v = property("X-KDE-IsAlso");
  return v.toString();
}

bool KMimeType::is( const QString& mimeTypeName ) const
{
  if ( name() == mimeTypeName )
      return true;
  QString st = parentMimeType();
  //if (st.isEmpty()) kDebug(7009)<<"Parent mimetype is empty"<<endl;
  while ( !st.isEmpty() )
  {
      //kDebug(7009)<<"Checking parent mime type: "<<st<<endl;
      KMimeType::Ptr ptr = KMimeType::mimeType( st );
      if (!ptr) return false; //error
      if ( ptr->name() == mimeTypeName )
          return true;
      st = ptr->parentMimeType();
  }
  return false;
}

int KMimeType::patternsAccuracy() const {
  QVariant v = property("X-KDE-PatternsAccuracy");
  if (!v.isValid()) return 100;
  else
      return v.toInt();
}


/*******************************************************
 *
 * KFolderType
 *
 ******************************************************/

/*
QString KFolderType::icon( const QString& _url, bool _is_local ) const
{
  if ( !_is_local || _url.isEmpty() )
    return KMimeType::icon( _url, _is_local );

  return KFolderType::icon( KUrl(_url) );
}
*/

QString KFolderType::icon( const KUrl& _url ) const
{
  if ( _url.isEmpty() || !_url.isLocalFile() )
    return KMimeType::iconName( _url );

  KUrl u( _url );
  u.addPath( ".directory" );

  QString icon;
  // using KStandardDirs as this one checks for path being
  // a file instead of a directory
  if ( KStandardDirs::exists( u.path() ) )
  {
    KSimpleConfig cfg( u.path(), true );
    cfg.setDesktopGroup();
    icon = cfg.readEntry( "Icon" );
    QString empty_icon = cfg.readEntry( "EmptyIcon" );

    if ( !empty_icon.isEmpty() )
    {
      bool isempty = false;
      DIR *dp = 0L;
      struct dirent *ep;
      dp = opendir( QFile::encodeName(_url.path()) );
      if ( dp )
      {
        QSet<QByteArray> entries;
        // Note that readdir isn't guaranteed to return "." and ".." first (#79826)
        ep=readdir( dp ); if ( ep ) entries.insert( ep->d_name );
        ep=readdir( dp ); if ( ep ) entries.insert( ep->d_name );
        if ( (ep=readdir( dp )) == 0L ) // third file is NULL entry -> empty directory
          isempty = true;
        else {
          entries.insert( ep->d_name );
          if ( readdir( dp ) == 0 ) { // only three
            // check if we got "." ".." and ".directory"
            isempty = entries.contains( "." ) &&
                      entries.contains( ".." ) &&
                      entries.contains( ".directory" );
          }
        }
        if (!isempty && !strcmp(ep->d_name, ".directory"))
          isempty = (readdir(dp) == 0L);
        closedir( dp );
      }

      if ( isempty )
        return empty_icon;
    }
  }

  if ( icon.isEmpty() )
    return KMimeType::iconName( _url );

  if ( icon.startsWith( "./" ) ) {
    // path is relative with respect to the location
    // of the .directory file (#73463)
    KUrl v( _url );
    v.addPath( icon.mid( 2 ) );
    icon = v.path();
  }

  return icon;
}

/*
QString KFolderType::comment( const QString& _url, bool _is_local ) const
{
  if ( !_is_local || _url.isEmpty() )
    return KMimeType::comment( _url, _is_local );

  return KFolderType::comment( KUrl(_url), _is_local );
}
*/

QString KFolderType::comment( const KUrl& _url ) const
{
  if ( _url.isEmpty() || !_url.isLocalFile() )
    return KMimeType::comment( _url );

  KUrl u( _url );
  u.addPath( ".directory" );

  KDesktopFile cfg( u.path(), true );
  QString comment = cfg.readComment();
  if ( comment.isEmpty() )
    return KMimeType::comment( _url );

  return comment;
}

const QString & KMimeType::defaultMimeType()
{
    static const QString & s_strDefaultMimeType =
        KGlobal::staticQString( "application/octet-stream" );
    return s_strDefaultMimeType;
}

void KMimeType::virtual_hook( int id, void* data )
{ KServiceType::virtual_hook( id, data ); }

void KFolderType::virtual_hook( int id, void* data )
{ KMimeType::virtual_hook( id, data ); }

#include "kmimetyperesolver.moc"

