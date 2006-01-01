/*
* kimageio.h -- Declaration of interface to the KDE Image IO library.
* Sirtaj Singh Kang <taj@kde.org>, 23 Sep 1998.
*
* This library is distributed under the conditions of the GNU LGPL.
*/

#ifndef SSK_KIMGIO_H
#define SSK_KIMGIO_H

#include <qstringlist.h>

#include <kdelibs_export.h>

/**
 * Interface to the KDE Image IO plugin architecture.
 *
 * This library allows KDE applications to read and write images in a
 * variety of formats, transparently via the QImage and QPixmap load
 * and save methods.
 *
 * The image processing backends are written as image handlers compatible
 * with the QImageIO handler format. The backends are loaded on demand
 * when a particular format is requested. Each format can be identified
 * by a unique type id string.
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
 * \b Usage:
 *
 * Simply call the KImageIO::registerFormats() static method declared
 * in kimageio.h.
 *
 * \b Example:
 *
 * \code
 * #include<qpixmap.h>
 * #include<kimageio.h>
 *
 * int main( int argc, char **argv )
 *  {
 *   ....
 *   KImageIO::registerFormats();
 *   ...   // start main program
 * }
 * \endcode
 *
 * @see KImageIO, QPixmap, QImage, QImageIO
 * @author Sirtaj Singh Kang
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
   *  Registers all KImageIO supported formats.
   */
  static void registerFormats();

  /**
   * Checks if a special type is supported for writing.
   * @param type the type id of the image type
   * @return true if the image format can be written
   */
  static bool canWrite(const QString& type);

  /**
   * Checks if a special type is supported for reading.
   * @param type the type id of the image type
   * @return true if the image format can be read
   */
  static bool canRead(const QString& type);

  /**
   * Returns a list of all KImageIO supported formats.
   *
   * @param mode Tells whether to retrieve modes that can be read or written.
   * @return a list of the type ids
   */
  static QStringList types(Mode mode = Writing);


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
   * Returns the suffix of an image type.
   * @param type the type id of the file format
   * @return the suffix of the file format or QString() if it does not
   *         exist
   */
  static QString suffix(const QString& type);

  /**
   * Returns the type of a MIME type.
   * @param mimeType the MIME type to search
   * @return type id of the MIME type or QString() if the MIME type
   *         is not supported
   * @since 3.1
   */
  static QString typeForMime(const QString& mimeType);

  /**
   * Returns the type of given filename.
   * @param filename the filename to check
   * @return if the file name's suffix is known the type id of the
   *         file type, otherwise QString()
   */
  static QString type(const QString& filename);

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

  /**
   * Returns the MIME type of @p _filename.
   * @param _filename the filename to check
   * @return the MIME type of the file, or QString()
   **/
  static QString mimeType( const QString& _filename );
};


#endif

