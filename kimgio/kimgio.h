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
* Registers all available image format encoders and decoders.
*/
void kimgioRegister();

/**
* Interface to the KDE Image IO library.
*
* Just call KImageIO::registerFormats to register all extra
* image formats provided by libkimgio. You will also need to
* link to this library.
*
* @author Sirtaj Singh Kang <taj@kde.org>
* @version $Id$
*/
class KImageIO
{

public:

  /**
   *  Registers all KIMGIO supported formats
   */
  static void registerFormats() { kimgioRegister(); }

  /**
   *  Checks if a special type is supported for writing.
   */
  static bool canWrite(const QString& type);

  /**
   *  Checks if a special type is supported for reading.
   */
  static bool canRead(const QString& type);

  enum Mode { Reading, Writing };

  /**
   *  Dont know.
   */
  static QStringList types(Mode mode = Writing);

  /**
   *  Retrieves a list of patterns of all KIMGIO supported formats.
   *
   *  @param mode Possible values are Reading (default) and Writing.
   */
  static QString pattern(Mode mode = Reading);

  /**
   *  Retrieves the suffix of an image type.
   */
  static QString suffix(const QString& type);

  /**
   *  Returns the type of given filename.
   */
  static QString type(const QString& filename);
};

/**
 * @libdoc The KDE Image I/O Library
 *
 * This library allows KDE applications to read and write images in a
 * variety of formats, transparently via the @ref QImage and QPixmap load
 * and save methods.
 *
 * The image processing backends are written as image handlers compatible
 * with the @ref QImageIO handler format. The backends are loaded on demand
 * when a particular format is requested.
 * 
 * @sect Usage
 *
 * Simply call the @ref KImageIO::registerFormats static method declared
 * in kimgio.h, and link libkimgio into your project.
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

#endif
