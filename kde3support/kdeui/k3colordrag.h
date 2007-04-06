/* This file is part of the KDE libraries
   Copyright (C) 1999 Steffen Hansen (hansen@kde.org)

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _K3COLORDRAG_H
#define _K3COLORDRAG_H

#include <kde3support_export.h>

#include <Qt3Support/Q3ColorDrag>
#include <QtGui/QColor>

class K3ColorDragPrivate;
/**
 * A drag-and-drop object for colors. The according MIME type
 * is set to application/x-color.
 *
 * See the Qt drag'n'drop documentation.
 */
class KDE3SUPPORT_EXPORT K3ColorDrag : public Q3StoredDrag {
    Q_OBJECT

public:
    /**
     * Constructs a color drag with a white color.
     */
    K3ColorDrag( QWidget *dragsource = 0, const char *name = 0 );
    /**
     * Constructs a color drag with the color @p col.
     */
    K3ColorDrag( const QColor &col, QWidget *dragsource = 0, const char *name = 0 );
    virtual ~K3ColorDrag() {}

    virtual const char *format(int i) const;
    virtual QByteArray encodedData ( const char * m ) const;

    /**
     * Sets the color of the drag to @p col.
     */
    void setColor(const QColor &col);
    /**
     * Returns true if the MIME source @p e contains a color object.
     */
    static bool canDecode(QMimeSource *e);
    /**
     * Decodes the MIME source @p e and puts the resulting color into @p col.
     */
    static bool decode(QMimeSource *e, QColor &col);
    /**
     * @deprecated This is equivalent with "new K3ColorDrag(color, dragsource)".
     */
    static K3ColorDrag* makeDrag( const QColor&,QWidget *dragsource);

private:
     QColor m_color; // unused
protected:
     virtual void virtual_hook( int id, void* data );
private:
     K3ColorDragPrivate *d;
};


#endif // _KCOLORDRAG_H
