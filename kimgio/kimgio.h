/**
* kimgio.h -- Declaration of interface to the KDE Image IO library.
* Sirtaj Singh Kang <taj@kde.org>, 23 Sep 1998.
*
* $Id$
*
* This library is distributed under the conditions of the GNU LGPL.
*/

#ifndef SSK_KIMGIO_H
#define SSK_KIMGIO_H

/**
* Registers all available image format encoders and decoders.
*/
void kimgioRegister(void);

/**
* Interface to the KDE Image IO library.
* @author Sirtaj Singh Kang <taj@kde.org>
* @version $Id$
*/
class KImageIO
{

public:

static void registerFormats() { kimgioRegister(); }

};

#endif
