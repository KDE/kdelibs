/* This file is part of the KDE project
   Copyright (C) 2001-2005 Christoph Cullmann (cullmann@kde.org)

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

#ifndef __ktexteditor_cursorinterface_h__
#define __ktexteditor_cursorinterface_h__

#include <kdelibs_export.h>

#include <QPoint>

namespace KTextEditor
{

class View;

/**
 * This is an interface to access the text cursor of a KTextEditor::View object.
 * It allows to set the cursor position or to locate it
 * Modifications will be signaled
 */
class KTEXTEDITOR_EXPORT CursorInterface
{
  public:
    /**
     * Virtual Destructor
     */
    virtual ~CursorInterface () {}

  public:
    /**
     * Set the cursor position
     * @param line new cursor line
     * @param col new cursor column, tabs count as MULTIPLE chars
     * @return success
     */
    virtual bool setCursorPosition (int line, int col) = 0;

    /**
     * Set the cursor position
     * @param line new cursor line
     * @param col new cursor column, tabs count as ONE char
     * @return success
     */
    virtual bool setCursorPositionReal (int line, int col) = 0;

    /**
     * Get the cursor position
     * @param line cursor line
     * @param col cursor column, tabs count as MULTIPLE chars
     */
    virtual void cursorPosition (int &line, int &col) const = 0;

    /**
     * Get the cursor position
     * @param line cursor line
     * @param col cursor column, tabs count as ONE char
     */
    virtual void cursorPositionReal (int &line, int &col) const = 0;

    /**
     * Get the cursor line
     * @return line cursor line
     */
    virtual int cursorLine () const = 0;

    /**
     * Get the cursor column
     * @return cursor column, tabs count as MULTIPLE chars
     */
    virtual int cursorColumn () const = 0;

    /**
     * Get the cursor column
     * @return cursor column, tabs count as ONE char
     */
    virtual int cursorColumnReal () const = 0;

    /**
     * Get the current cursor coordinates in pixels.
     * @return cursor screen coordinates
     */
    virtual QPoint cursorCoordinates () const = 0;

  /**
   * SIGNALS
   * following signals should be emitted by the editor view
   * if the cursor position changes
   */
  private:
    /**
     * cursor position changed!
     */
    virtual void cursorPositionChanged () = 0;
};

KTEXTEDITOR_EXPORT CursorInterface *cursorInterface (View *view);

}

#endif
