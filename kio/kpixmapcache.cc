#include "kmimetypes.h"

#include <qpixmapcache.h>
#include <qpixmap.h>

#include <qstring.h>

#include <kapp.h>

#include <sys/stat.h>

#include "kpixmapcache.h"
#include <kstddirs.h>
#include <kglobal.h>

QPixmap* KPixmapCache::pixmapForURL( const KURL& _url, mode_t _mode, bool _is_local_file, bool _mini )
{
  return pixmap( KMimeType::findByURL( _url, _mode, _is_local_file )->icon( _url, _is_local_file ).ascii(), _mini );
}

QPixmap* KPixmapCache::pixmapForURL( const char* _url, mode_t _mode, bool _is_local_file, bool _mini )
{
  KURL url( _url );
  
  return pixmap( KMimeType::findByURL( url, _mode, _is_local_file )->icon( url, _is_local_file ).ascii(), _mini );
}

QString KPixmapCache::pixmapFileForURL( const char* _url, mode_t _mode, bool _is_local_file, bool _mini )
{
  KURL url( _url );
  
  return pixmapFile( KMimeType::findByURL( url, _mode, _is_local_file )->icon( url, _is_local_file ).ascii(), _mini );
}

QPixmap* KPixmapCache::pixmapForMimeType( const char *_mime_type, bool _mini )
{
  return pixmap( KMimeType::mimeType( _mime_type )->icon( QString::null, false ).ascii(), _mini );
}

QPixmap* KPixmapCache::pixmapForMimeType( KMimeType *_mime_type, bool _mini )
{
  return pixmap( _mime_type->icon( QString::null, false ).ascii(), _mini );
}

QPixmap* KPixmapCache::pixmapForMimeType( KMimeType *_mime_type, const KURL& _url, bool _is_local_file, bool _mini )
{
  return pixmap( _mime_type->icon( _url, _is_local_file ).ascii(), _mini );
}

QString KPixmapCache::pixmapFileForMimeType( const char *_mime_type, bool _mini )
{
  return pixmapFile( KMimeType::mimeType( _mime_type )->icon( QString::null, false ).ascii(), _mini );
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

  QString file = locate("wallpaper", _wallpaper);
  
  QPixmap p1;
  p1.load( file );
  if ( !p1.isNull() )
  {
    QPixmapCache::insert( key, p1 );
    return QPixmapCache::find( key );
  }

  return 0;
}

QPixmap* KPixmapCache::pixmap( const char *_pixmap, bool _mini )
{
  QString file = pixmapFile(_pixmap, _mini);

  QPixmap* pix = QPixmapCache::find( file );
  if ( pix )
    return pix;
  
  QPixmap p1;
  p1.load( file );
  if ( !p1.isNull() )
    QPixmapCache::insert( file, p1 );
  
  // there will always be an entry, as pixmapFile will return
  // unknown.xpm if in doubt
  return QPixmapCache::find( file );
}

QString KPixmapCache::pixmapFile( const char *_pixmap, bool _mini )
{
  QString key = _mini ? "mini" : "icon";
  QString file = locate(key, _pixmap);
  if (file.isNull())
    return locate(key, "unknown.xpm");
  
  return file;
}

QPixmap* KPixmapCache::defaultPixmap( bool _mini )
{
  return pixmap( "unknown.xpm", _mini );
}
