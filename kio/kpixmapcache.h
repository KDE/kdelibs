#ifndef __kpixmapcache_h__
#define __kpixmapcache_h__

#include <sys/stat.h>

#include <qstring.h>

#include <k2url.h>

class QPixmap;
class KMimeType;

class KPixmapCache
{
public:
  /**
   * May return 0L if the pixmap and the default pixmap is not available
   */
  static QPixmap* pixmapForURL( K2URL& _url, mode_t _mode = 0, bool _is_local_file = false, bool _mini = false );
  /**
   * May return 0L if the pixmap and the default pixmap is not available
   */
  static QPixmap* pixmapForURL( const char *_url, mode_t _mode = 0, bool _is_local_file = false, bool _mini = false );
  /**
   * Never returns an empty string. If the image was not found, then the default pixmap is returned.
   *
   * @return the full path of the requested pixmap.
   */
  static QString pixmapFileForURL( const char *_url, mode_t _mode = 0, bool _is_local_file = false, bool _mini = false );
  /**
   * May return 0L if the pixmap and the default pixmap is not available
   * Use this function only if you dont have a special URL for which you search a pixmap.
   * This functions is useful to fond out, which icon is usually chosen for a certain mime type.
   * Since no URL is passed, it is impossible to obeye icon hints in kdelnk files for example.
   */
  static QPixmap* pixmapForMimeType( const char *_mime_type, bool _mini );
  /**
   * May return 0L if the pixmap and the default pixmap is not available.
   * Use this function only if you dont have a special URL for which you search a pixmap.
   * This functions is useful to fond out, which icon is usually chosen for a certain mime type.
   * Since no URL is passed, it is impossible to obeye icon hints in kdelnk files for example.
   */
  static QPixmap* pixmapForMimeType( KMimeType *_mime_type, bool _mini );
  /**
   * May return 0L if the pixmap and the default pixmap is not available
   */
  static QPixmap* pixmapForMimeType( KMimeType *_mime_type, K2URL& _url, bool _is_local_file, bool _mini );
  /**
   * Never returns an empty string. If the image was not found, then the default pixmap is returned.
   *
   * @return the full path of the requested pixmap.
   */
  static QString pixmapFileForMimeType( const char *_mime_type, bool _mini );
  /**
   * May return 0L if the pixmap and the default pixmap is not available
   */
  static QPixmap* pixmap( const char *_pixmap, bool _mini );
  /**
   * @return the full path of the requested icon.
   */
  static QString pixmapFile( const char *_pixmap, bool _mini );
  /**
   * May return 0L if the pixmap is not available
   */
  static QPixmap* defaultPixmap( bool _mini );
  /**
   * May return 0L if the pixmap is not available
   */
  static QPixmap* toolbarPixmap( const char *_pixmap );
  /**
   * May return 0L if the pixmap is not available
   */
  static QPixmap* wallpaperPixmap( const char *_wallpaper );
};

#endif
