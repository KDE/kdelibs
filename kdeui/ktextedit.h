/* This file is part of the KDE libraries
    Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

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

#ifndef KTEXTEDIT_H
#define KTEXTEDIT_H

#include <qtextedit.h>

/**
 * This is just a little subclass of QTextEdit, implementing
 * some standard KDE features, like Cursor auto-hiding, configurable
 * wheelscrolling (fast-scroll or zoom) and deleting of entire
 * words with Ctrl-Backspace or Ctrl-Delete.
 *
 * Basic rule: whenever you want to use QTextEdit, use KTextEdit!
 *
 * @see QTextEdit
 * @short A KDE'ified QTextEdit
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KTextEdit : public QTextEdit
{
    Q_OBJECT
    
public:
    /**
     * Constructs a KTextEdit object. See @ref QTextEdit::QTextEdit
     * for details.
     */
    KTextEdit( const QString& text,
               const QString& context = QString::null, 
               QWidget * parent = 0, const char *name = 0 );
    /**
     * Constructs a KTextEdit object. See @ref QTextEdit::QTextEdit
     * for details.
     */

    KTextEdit( QWidget *parent = 0L, const char *name = 0 );
    /**
     * Destroys the KTextEdit object.
     */
    ~KTextEdit();
    
    /**
     * Reimplemented to set a proper "deactivated" background color.
     */
    virtual void setReadOnly (bool readOnly);

protected:
    /**
     * Reimplemented to catch "delete word" key events.
     */
    virtual void keyPressEvent( QKeyEvent * );

    /**
     * Reimplemented to allow fast-wheelscrolling with Ctrl-Wheel
     * or zoom.
     */
    virtual void contentsWheelEvent( QWheelEvent * );

    /**
     * Deletes a word backwards from the current cursor position,
     * if available.
     */
    virtual void deleteWordBack();

    /**
     * Deletes a word forwards from the current cursor position,
     * if available.
     */
    virtual void deleteWordForward();
    
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KTextEditPrivate;
    KTextEditPrivate *d;

};

#endif // KTEXTEDIT_H
