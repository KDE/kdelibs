#include "kmimetypes.h"

#include <qpixmapcache.h>
#include <qpixmap.h>

#include <qstring.h>

#include <kapp.h>

#include <sys/stat.h>

#include "kpixmapcache.h"

QPixmap* KPixmapCache::pixmapForURL( KURL& _url, mode_t _mode, bool _is_local_file, bool _mini )
{
  return pixmap( KMimeType::findByURL( _url, _mode, _is_local_file )->icon( _url, _is_local_file ), _mini );
}

QPixmap* KPixmapCache::pixmapForURL( const char* _url, mode_t _mode, bool _is_local_file, bool _mini )
{
  KURL url( _url );
  
  return pixmap( KMimeType::findByURL( url, _mode, _is_local_file )->icon( url, _is_local_file ), _mini );
}

QString KPixmapCache::pixmapFileForURL( const char* _url, mode_t _mode, bool _is_local_file, bool _mini )
{
  KURL url( _url );
  
  return pixmapFile( KMimeType::findByURL( url, _mode, _is_local_file )->icon( url, _is_local_file ), _mini );
}

QPixmap* KPixmapCache::pixmapForMimeType( const char *_mime_type, bool _mini )
{
  return pixmap( KMimeType::find( _mime_type )->icon( 0L, false ), _mini );
}

QPixmap* KPixmapCache::pixmapForMimeType( KMimeType *_mime_type, bool _mini )
{
  return pixmap( _mime_type->icon( 0L, false ), _mini );
}

QPixmap* KPixmapCache::pixmapForMimeType( KMimeType *_mime_type, KURL& _url, bool _is_local_file, bool _mini )
{
  return pixmap( _mime_type->icon( _url, _is_local_file ), _mini );
}

QString KPixmapCache::pixmapFileForMimeType( const char *_mime_type, bool _mini )
{
  return pixmapFile( KMimeType::find( _mime_type )->icon( 0L, false ), _mini );
}

QPixmap* KPixmapCache::toolbarPixmap( const char *_pixmap )
{
  QString key = "toolbar/";
  key += _pixmap;

  QPixmap* pix = QPixmapCache::find( key );
  if ( pix )
    return pix;

  QString file = kapp->localkdedir().data();
  file += "/share/toolbar/";
  file += _pixmap;
  
  QPixmap p1;
  p1.load( file );
  if ( !p1.isNull() )
  {
    QPixmapCache::insert( key, p1 );
    return QPixmapCache::find( key );
  }

  file = kapp->kde_toolbardir().data();
  file += "/";
  file += _pixmap;

  p1.load( file );
  if ( !p1.isNull() )
  {
    QPixmapCache::insert( key, p1 );
    return QPixmapCache::find( key );
  }
  
  return 0L;
}

QPixmap* KPixmapCache::wallpaperPixmap( const char *_wallpaper )
{
  QString key = "wallpapers/";
  key += _wallpaper;

  QPixmap* pix = QPixmapCache::find( key );
  if ( pix )
    return pix;

  QString file = kapp->localkdedir().data();
  file += "/share/wallpapers/";
  file += _wallpaper;
  
  QPixmap p1;
  p1.load( file );
  if ( !p1.isNull() )
  {
    QPixmapCache::insert( key, p1 );
    return QPixmapCache::find( key );
  }

  file = kapp->kde_wallpaperdir().data();
  file += "/";
  file += _wallpaper;

  p1.load( file );
  if ( !p1.isNull() )
  {
    QPixmapCache::insert( key, p1 );
    return QPixmapCache::find( key );
  }
  
  return 0L;
}

QPixmap* KPixmapCache::pixmap( const char *_pixmap, bool _mini )
{
  QString key = "";
  if ( _mini )
    key = "mini/";
  key += _pixmap;
  
  QPixmap* pix = QPixmapCache::find( key );
  if ( pix )
    return pix;
  
  QString file = kapp->localkdedir().data();
  file += "/share/icons/";
  file += key;
  
  QPixmap p1;
  p1.load( file );
  if ( !p1.isNull() )
  {
    QPixmapCache::insert( key, p1 );
    return QPixmapCache::find( key );
  }

  file = kapp->kde_icondir().data();
  file += "/";
  file += key;
  
  p1.load( file );
  if ( !p1.isNull() )
  {
    QPixmapCache::insert( key, p1 );
    return QPixmapCache::find( key );
  }

  if ( strcmp( _pixmap, "unknown.xpm" ) == 0 )
    return 0L;
  
  return defaultPixmap( _mini );
}

QString KPixmapCache::pixmapFile( const char *_pixmap, bool _mini )
{
  QString key = "";
  if ( _mini )
    key = "mini/";
  key += _pixmap;
  
  QString file = kapp->localkdedir().data();
  file += "/share/icons/";
  file += key;

  struct stat buff;
  if ( stat( file, &buff ) != -1 )
    return file;
  
  file = kapp->kde_icondir().data();
  file += "/";
  file += key;

  if ( stat( file, &buff ) != -1 )
    return file;

  file = kapp->kde_icondir().data();
  file += "/";
  if ( _mini )
    file += "mini/";
  file += "unknown.xpm";

  return file;
}

QPixmap* KPixmapCache::defaultPixmap( bool _mini )
{
  return pixmap( "unknown.xpm", _mini );
}
