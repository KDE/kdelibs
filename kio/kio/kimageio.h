/*
* kimageio.h -- Declaration of interface to the KDE Image IO library.
* Sirtaj Singh Kang <taj@kde.org>, 23 Sep 1998.
*
* This library is distributed under the conditions of the GNU LGPL.
*/

#ifndef SSK_KIMGIO_H
#define SSK_KIMGIO_H

#include <QStringList>
#include <QString>

#include <kdelibs_export.h>

/**
 * A small convenience class to get information about image format names and
 * the corresponding mime type. Also it can give information about supported
 * image types without loading all the imageformat plugins.
 *
 * The image processing backends are written as image handlers compatible
 * with the QImageIOHandler format. The backends are Qt imageformat plugins. 
 * Each format can be identified by a unique type id string.
 *
 * \b Formats:
 *
 * Currently supported formats include:
 * @li BMP     \<read\> \<write\>
 * @li EPS     \<read\> \<write\>
 * @li EXR     \<read\>
 * @li G3      \<read\>
 * @li GIF     \<read\>
 * @li ICO     \<read\>
 * @li JP2     \<read\> \<write\>
 * @li JPEG    \<read\> \<write\>
 * @li NETPBM  \<read\> \<write\>
 * @li PCX     \<read\> \<write\>
 * @li PNG     \<read\> \<write, only with newer libraries\>
 * @li TGA     \<read\> \<write\>
 * @li TIFF    \<read\>
 * @li XBM     \<read\> \<write\>
 * @li XPM     \<read\> \<write\>
 * @li XV      \<read\> \<write\>
 *
 */
class KIO_EXPORT KImageIO
{
public:
  /**
   * Possible image file access modes.
   *
   * Used in various KImageIO static function.
   **/
  enum Mode { Reading, Writing };

  /**
   * Returns a list of patterns of all KImageIO supported formats.
   *
   * These patterns can be passed to KFileDialog::getOpenFileName()
   * or KFileDialog::getSaveFileName(), for example.
   *
   * @param mode Tells whether to retrieve modes that can be read or written.
   * @return a space-separated list of file globs that describe the
   * supported formats
   */
  static QString pattern(Mode mode = Reading);

  /**
   * Returns the type of a MIME type.
   * @param mimeType the MIME type to search
   * @return type id(s) of the MIME type or QStringList() if the MIME type
   *         is not supported
   * @since 3.1
   */
  static QStringList typeForMime(const QString& mimeType);
  /**
   * Returns a list of all KImageIO supported formats.
   *
   * @param mode Tells whether to retrieve modes that can be read or written.
   * @return a list of the type ids
   */
  static QStringList types(Mode mode = Writing);
                   
  /**
   *  Returns a list of MIME types for all KImageIO supported formats.
   *
   * @param mode Tells whether to retrieve modes that can be read or written.
   * @return a list if MIME types of the supported formats
   */
  static QStringList mimeTypes( Mode mode = Writing );

  /**
   * Test to see whether a MIME type is supported to reading/writing.
   * @param _mimeType the MIME type to check
   * @param _mode Tells whether to check for reading or writing capabilities
   * @return true if the type is supported
   **/
  static bool isSupported( const QString& _mimeType, Mode _mode = Writing );
};


#endif

