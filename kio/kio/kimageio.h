/*
* kimgio.h -- Declaration of interface to the KDE Image IO library.
* Sirtaj Singh Kang <taj@kde.org>, 23 Sep 1998.
*
* $Id$
*
* This library is distributed under the conditions of the GNU LGPL.
*/

#ifndef SSK_KIMGIO_H
#define SSK_KIMGIO_H

#include <qstringlist.h>

/**
 * Interface to the KDE Image IO plugin architecture.
 *
 * This library allows KDE applications to read and write images in a
 * variety of formats, transparently via the @ref QImage and @ref QPixmap load
 * and save methods.
 *
 * The image processing backends are written as image handlers compatible
 * with the @ref QImageIO handler format. The backends are loaded on demand
 * when a particular format is requested. Each format can be identified
 * by a unique type id string.
 * 
 * @sect Formats
 *
 * Currently supported formats include:
 * @li JPEG    <read>
 * @li XV      <read> <write>
 * @li EPS     <read> <write>
 * @li NETPBM  <incomplete>
 * @li PNG     <read> <write, only with newer libraries>
 * @li TIFF    <read>
 * @li KRL     <read> 
 *
 * @sect Usage
 *
 * Simply call the @ref KImageIO::registerFormats() static method declared
 * in kimgageio.h.
 * 
 * @sect Example
 *
 * <pre>
	#include<qpixmap.h>
 	#include<kimgio.h>

	int main( int argc, char **argv )
	{
		....
		KImageIO::registerFormats();
		
		...	// start main program
	}
 * </pre>
 *
 * @see KImageIO, QPixmap, QImage, QImageIO
 * @author Sirtaj Singh Kang
 * @version $Id$
 */
class KImageIO
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
   * @param the type id of the image type
   * @return true if the image format can be written
   */
  static bool canWrite(const QString& type);

  /**
   * Checks if a special type is supported for reading.
   * @param the type id of the image type
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
   * These patterns can be passed to @ref KFileDialog::getOpenFileName()
   * or @ref KFileDialog::getSaveFileName(), for example.
   *
   * @param mode Tells whether to retrieve modes that can be read or written.
   */
  static QString pattern(Mode mode = Reading);

  /**
   * Returns the suffix of an image type.
   * @param type the type id of the file format
   * @return the suffix of the file format or QString::null if it does not
   *         exist
   */
  static QString suffix(const QString& type);

  /**
   * Returns the type of a MIME type.
   * @param mimeType the MIME type to search
   * @return type id of the MIME type or QString::null if the MIME type
   *         is not supported
   */
  static QString typeForMime(const QString& mimeType);

  /**
   * Returns the type of given filename.
   * @param filename the filename to check
   * @return if the file name's suffix is known the type id of the
   *         file type, otherwise QString::null
   */
  static QString type(const QString& filename);

  /**
   *  Returns a list of MIME types for all KImageIO supported formats. 
   *
   * @param mode Tells whether to retrieve modes that can be read or written.
   * @return a list if MIME types of the supported formats
   */
  static QStringList mimeTypes( Mode _mode = Writing );

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
   * @return the MIME type of the file, or QString::null
   **/
  static QString mimeType( const QString& _filename );
};


#endif

