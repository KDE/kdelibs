/* This file is part of the KDE libraries
   Copyright (C) 1999 Steffen Hansen (hansen@kde.org)
   Copyright (C) 2005 Joseph Wenninger (jowenn@kde.org)
   
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

#ifndef _KCOLORMIMEDATA_H
#define _KCOLORMIMEDATA_H

#include <qcolor.h>
#include <kdelibs_export.h>

class QMimeData;
class QDrag;
/**
 * Drag-and-drop and clipboard mimedata manipulation for QColor objects. The according MIME type
 * is set to application/x-color.
 *
 * See the Qt drag'n'drop documentation.
 */
class KDEUI_EXPORT KColorMimeData {
public:
    /**
     * Sets the color and text representation fields for the specified color in the mimedata object
     * ==> application/x-color and text/plain types are  set
     */
    static void populateMimeData(QMimeData *mimeData,const QColor& color);

    /**
     * Returns true if the MIME data @p mimeData contains a color object.
     * First checks for application/x-color and if that failes for a text/plain entry, which
     * represents a color in the format #hexnumbers
     */
    static bool canDecode(const QMimeData *mimeData);

    /**
     * Decodes the MIME data @p mimeData and puts the resulting color into @p col.
     * First tries application/x-color and if that failes  a text/plain entry, which
     * represents a color in the format #hexnumbers. If this fails to an invalid QColor objects
     * is returned @see QColor::isValid()
     */
    static QColor fromMimeData(const QMimeData *mimeData);
    
    /**
     * creates a color drag object. Either you have to start this drag or delete it
     * The drag object's mime data has the application/x-color and text/plain type set and a pixmap
     * filled with the specifiec color, which is going to be displayed next to the mouse cursor
     */
    static QDrag* createDrag( const QColor& color,QWidget *dragsource);
};


#endif // _KCOLORMIMEDATA_H
