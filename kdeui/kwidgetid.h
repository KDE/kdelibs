/*
    This file is part of the KDE libraries

    Copyright (C) 1998 René Beutler (rbeutler@g26.ethz.ch)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef __HELP_H
#define __HELP_H

#include <qobject.h>

/** @short KDE Help Center Widget Identification
 *
 *  This provides you with an easy way to make your widget identifieable by *every*
 *  KDE application running on the same screen, independant from user/machine.
 *
 *  This means
 *  a) given the widget identifier, it is possible to get it's geometry, making it easy
 *     for the help system to show the user where he can find your widgets
 *  b) given a point inside your widget (e.g. by mouseclick) you can get the identifier.
 *     this can be used to get help on widgets or go directly to the appropriate
 *     control center page.
 *
 *  Format of widget indentifiers:
 *     <appname>/<widgetname>[#<optionalName]
 *
 *  This will be translated by the Help Center to the following URL:
 *     kde_htmldir()/<language>/<appname>/widgets/<widgetname>.html
 *
 *  Everything after an optional '#' will be excluded in the URL. This is useful if you
 *  have several buttons which do the same, e.g the buttons to change the desktop in kpanel.
 *  You don't need to write a help file for every button, but in the sametime you can
 *  distinguish them.
 *
 *  example: kpanel/changeDesktopButton#0
 *           the corresponding help file will be $KDEDIR/share/doc/HTML/<language>/kpanel/widgets/changeDesktopButton.html
 */

/**
 *   Associates _widget with the identifier _str.
 */

void setWidgetIdentifier( QObject *_widget, const char *_str )
{
	if( ( _widget != 0L ) && (_widget->isWidgetType() == true) )
	{
		((QWidget *)_widget)->setCaption( _str );
	}
}

#endif
