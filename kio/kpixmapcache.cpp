#include "kmimetype.h"

#include <qpixmapcache.h>
#include <qpixmap.h>

#include <qstring.h>

#include <kapp.h>

#include <sys/stat.h>

#include "kpixmapcache.h"
#include <kstddirs.h>
#include <kglobal.h>
#include <kiconloader.h>

QPixmap KPixmapCache::pixmapForURL( const KURL& _url, mode_t _mode, bool _is_local_file, bool _mini )
{
  return pixmap( KMimeType::findByURL( _url, _mode, _is_local_file )->icon( _url, _is_local_file ).ascii(), _mini );
}

QPixmap KPixmapCache::pixmapForURL( const char* _url, mode_t _mode, bool _is_local_file, bool _mini )
{
  KURL url( _url );
  
  return pixmap( KMimeType::findByURL( url, _mode, _is_local_file )->icon( url, _is_local_file ).ascii(), _mini );
}

QString KPixmapCache::pixmapFileForURL( const char* _url, mode_t _mode, bool _is_local_file, bool _mini )
{
  KURL url( _url );
  
  return pixmapFile( KMimeType::findByURL( url, _mode, _is_local_file )->icon( url, _is_local_file ).ascii(), _mini );
}

QPixmap KPixmapCache::pixmapForMimeType( const char *_mime_type, bool _mini )
{
  return pixmap( KMimeType::mimeType( _mime_type )->icon( QString::null, false ).ascii(), _mini );
}

QPixmap KPixmapCache::pixmapForMimeType( KMimeType::Ptr _mime_type, bool _mini )
{
  return pixmap( _mime_type->icon( QString::null, false ).ascii(), _mini );
}

QPixmap KPixmapCache::pixmapForMimeType( KMimeType::Ptr _mime_type, const KURL& _url, bool _is_local_file, bool _mini )
{
  return pixmap( _mime_type->icon( _url, _is_local_file ).ascii(), _mini );
}

QString KPixmapCache::pixmapFileForMimeType( const char *_mime_type, bool _mini )
{
  return pixmapFile( KMimeType::mimeType( _mime_type )->icon( QString::null, false ).ascii(), _mini );
}

QString hackLocate( const QString &type, const QString &filename )
{
  QString file = locate( type, filename );
  if ( file.isNull() && filename.right( 4 ) == ".xpm" )
  {
    file = filename;
    file.truncate( file.length() - 4 );
    file.append( ".png" );
    file = locate( type, file );
  }
  return file;
}

QPixmap hackCacheFind( const QString &key )
{
  QPixmap *pix = QPixmapCache::find( key );
  
  if ( !pix && key.right( 4 ) == ".xpm" )
  {
    QString key2 = key;
    key2.truncate( key2.length() - 4 );
    key2.append( ".png" );
    pix = QPixmapCache::find( key2 );
  }
  
  if (pix)
    return *pix;
  return QPixmap();
}

QPixmap KPixmapCache::wallpaperPixmap( const char *_wallpaper )
{
  QString key = "wallpapers/";
  key += _wallpaper;

  QPixmap pix = hackCacheFind( key );
  if ( !pix.isNull() )
    return pix;

  QString file = hackLocate("wallpaper", _wallpaper);
  
  QPixmap p1;
  p1.load( file );
  if ( !p1.isNull() )
  {
    QPixmapCache::insert( key, p1 );
    return *QPixmapCache::find( key );
  }

  return QPixmap();
}

QPixmap KPixmapCache::pixmap( const char *_pixmap, bool _mini )
{
  return KGlobal::iconLoader()->
    loadApplicationIcon(_pixmap, 
			_mini ? KIconLoader::Small : KIconLoader::Medium);
}

QString KPixmapCache::pixmapFile( const char *_pixmap, bool _mini )
{
  QString path;
  KGlobal::iconLoader()->
    loadApplicationIcon(_pixmap,
			_mini ? KIconLoader::Small : KIconLoader::Medium, &path);
  return path;
}


