/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef K3MULTIPLEDRAG_H
#define K3MULTIPLEDRAG_H

#ifndef QT_NO_DRAGANDDROP

#include <kde3support_export.h>
#include <Qt3Support/Q3DragObject>
#include <Qt3Support/Q3ValueList>

class K3MultipleDragPrivate;
/**
 * This class makes it easy for applications to provide a drag object
 * (for drag-n-drop or for clipboard) that has several representations
 * of the same data, under different formats.
 *
 * Instead of creating a specific class for each case (as would otherwise
 * be necessary), you can simply create independent drag objects (e.g.
 * a Q3ImageDrag object and a K3URLDrag object), and bundle them together
 * using K3MultipleDrag.
 *
 * Sample code for this:
 *
 * \code
 * K3MultipleDrag *drag = new K3MultipleDrag( parentWidget );
 * drag->addDragObject( new Q3ImageDrag( someQImage, 0 ) );
 * drag->addDragObject( new K3URLDrag( someKUrl, 0 ) );
 * drag->drag();
 * \endcode
 *
 * Note that the drag objects added to the multiple drag become owned by it.
 * For that reason their parent should be 0.
 *
 * @deprecated use QMimeData instead
 * @author David Faure <faure@kde.org>
 */
class KDE3SUPPORT_EXPORT K3MultipleDrag : public Q3DragObject
{
    Q_OBJECT

public:
    /**
     * Create a new K3MultipleDrag object.
     * @param dragSource the parent object which is the source of the data,
     *                   0 for a parent-less object
     * @param name the name of the object, can be 0
     */
    K3MultipleDrag( QWidget *dragSource = 0, const char *name = 0 );

    /**
     * Call this to add each underlying drag object to the multiple drag object.
     * The drag object should not have a parent because the multiple drag object
     * will own it.
     *
     * @param dragObject the drag object to add. Should have no parent object.
     */
    void addDragObject( Q3DragObject *dragObject );

    /**
     * Returns the data of a drag object with that supports the given
     * mime type.
     * @param mime the mime type to search
     * @return the data, or a null byte array if not found
     * @reimp
     */
    virtual QByteArray encodedData( const char *mime ) const;

    /**
     * Returns the @p i'th supported format, or 0.
     * @param i the number of the format to check
     * @return the format with the number @p i, or 0 otherwise
     * @reimp
     */
    virtual const char* format( int i ) const;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    Q3PtrList<Q3DragObject> m_dragObjects;
    QList<int> m_numberFormats;
    K3MultipleDragPrivate* d;
};

#endif // QT_NO_DRAGANDDROP

#endif // K3MULTIPLEDRAG_H
