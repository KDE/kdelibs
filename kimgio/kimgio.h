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

	static void registerFormats() { kimgioRegister(); }
	static bool canWrite(const QString& type);
	static bool canRead(const QString& type);

	enum Mode { Reading, Writing };
	static QStringList types(Mode mode = Writing);
	static QString pattern(Mode mode = Reading);

	static QString suffix(const QString& type);
	static QString type(const QString& filename);
};

#endif
