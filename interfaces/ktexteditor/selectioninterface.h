/* This file is part of the KDE project
   Copyright (C) 2001 Christoph Cullmann (cullmann@kde.org)

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

#ifndef __ktexteditor_selectioninterface_h__
#define __ktexteditor_selectioninterface_h__

#include <qstring.h>

#include <kdelibs_export.h>

namespace KTextEditor
{

class View;

/**
 * This is an interface for the selection of a KTextEditor::View object.
 * It allows to modify and query the selection state, turn block selection mode
 * on/off and trigger the copy'n'paste actions
 * Modifications will be signaled
 */
class KTEXTEDITOR_EXPORT SelectionInterface
{
  public:
    /**
     * Virtual Destructor
     */
    virtual ~SelectionInterface() {}

  public:
    /**
    *  @return set the selection from line_start,col_start to line_end,col_end
    */
    virtual bool setSelection ( int startLine, int startCol, int endLine, int endCol ) = 0;

    /**
    *  removes the current Selection (not Text)
    */
    virtual bool clearSelection () = 0;

    /**
    *  @return true if there is a selection
    */
    virtual bool hasSelection () const = 0;

    /**
    *  @return a QString for the selected text
    */
    virtual QString selection () const = 0;

    /**
    *  removes the selected Text
    */
    virtual bool removeSelectedText () = 0;

    /**
    *  select the whole text
    */
    virtual bool selectAll () = 0;

    /** The selection start line number */
    virtual int selectionStartLine () const = 0;

    /** The selection start col */
    virtual int selectionStartColumn () const = 0;

    /** The selection end line */
    virtual int selectionEndLine () const = 0;

    /** The selection end col */
    virtual int selectionEndColumn () const = 0;

  /**
   * Blockselection stuff
   */
  public:
   /**
    * Returns the status of the selection mode - true indicates block selection mode is on.
    * If this is true, selections applied via the SelectionInterface are handled as
    * blockselections and the copy'n'paste functions works on
    * rectangular blocks of text rather than normal.
    */
    virtual bool blockSelectionMode () const = 0;

    /**
    * Set block selection mode to state "on"
    */
    virtual bool setBlockSelectionMode (bool on) = 0;

  /**
   * Copy'n'Paste stuff
   */
  public:
   /**
    * copies selected text to clipboard
    */
    virtual void copy ( ) const = 0;

    /**
    * copies selected text
    */
    virtual void cut ( ) = 0;

    /**
    * copies selected text to clipboard
    */
    virtual void paste ( ) = 0;

	/**
   * SIGNALS
   * following signals should be emitted by the editor view
   * if the selection state changes
   * both on selection change itself and on change of blockselection mode!
   */
	private:
	  virtual void selectionChanged () = 0;
};

KTEXTEDITOR_EXPORT SelectionInterface *selectionInterface (View *view);

}

#endif
