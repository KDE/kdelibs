/* This file is part of the KDE libraries
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __ktexteditor_view_h__
#define __ktexteditor_view_h__

// the very important KTextEditor::Cursor class
#include <ktexteditor/cursor.h>

// gui merging
#include <kxmlguiclient.h>

// widget
#include <qwidget.h>

class QMenu;

namespace KTextEditor
{

class Document;

/**
 * The View class represents a single view of a KTextEditor::Document
 * The view should provide both the graphical representation of the text
 * and the xmlgui for the actions
 */
class KTEXTEDITOR_EXPORT View : public QWidget, public KXMLGUIClient
{
  Q_OBJECT

  public:
    /**
     * Create a new view to the given document. The document must be non-null.
     * @param document parent document
     * @param parent parent widget
     */
    View ( QWidget *parent );

    /**
     * virtual destructor
     */
    virtual ~View ();

    /**
     * Returns the global number of this view in your app.
     * @return view number
     */
    int viewNumber () const;

  /**
   * Accessor for the document
   */
  public:
    /**
     * Access the parent Document.
     * @return document
     */
    virtual Document *document () = 0;

  /**
   * General information about this view
   */
  public:
    /**
     * Current view state
     * This can be used for example to show up that this view is now
     * in INSERT mode, or OVERWRITE mode, or COMMAND mode, or whatever
     * The string should be i18n, as this is a user aimed representation
     * of the view state, which should be shown in the GUI
     */
    virtual QString viewMode () const = 0;

  /**
   * SIGNALS
   * following signals should be emitted by the editor view
   */
  private:
    /**
     * view got focus
     * @param view view which got focus
     */
    virtual void focusIn ( View *view ) = 0;

    /**
     * view lost focus
     * @param view view which lost focus
     */
    virtual void focusOut ( View *view ) = 0;

    /**
     * view mode changed
     * @param view view which changed mode
     */
    virtual void viewModeChanged ( View *view ) = 0;

  /**
   * Context menu handling
   */
  public:
    /**
     * Set a context menu for this view
     * @param menu new context menu object for this view
     */
    virtual void setContextMenu ( QMenu *menu ) = 0;

    /**
     * Retrieve the context menu for this view
     * @return context menu object for this view or 0
     */
    virtual QMenu *contextMenu () = 0;

  /**
   * Cursor handling
   */
  public:
    /**
     * Set the cursor position, position is in characters
     * @param position new cursor position
     * @return success
     */
    virtual bool setCursorPosition (const KTextEditor::Cursor &position) = 0;

    /**
     * Get the cursor position, position is in characters
     * @return cursor position
     */
    virtual const KTextEditor::Cursor &cursorPosition () const = 0;

    /**
     * Get the virtual cursor position
     * @return cursor position, tabs count as MULTIPLE chars, as configured by user
     * this allows access to the user visible values of the cursor position
     */
    virtual KTextEditor::Cursor cursorPositionVirtual () const = 0;

    /**
     * Get the screen coordinates of the cursor position
     * @return cursor screen coordinates
     */
    virtual QPoint cursorPositionCoordinates () const = 0;

  /**
   * SIGNALS
   * following signals should be emitted by the editor view
   * if the cursor position changes
   */
  private:
    /**
     * cursor position changed!
     * @param view view which emitted the signal
     */
    virtual void cursorPositionChanged (View *view) = 0;

  /**
   * Selection methodes
   * This deals with text selection & copy'n'paste
   */
  public:
    /**
     * Set selection of view, old selection will be discarded
     * @param startPosition start of the new selection
     * @param endPosition end of the new selection
     * @return success
     */
    virtual bool setSelection ( const Cursor &startPosition, const Cursor &endPosition ) = 0;

    /**
     * Is there any non-empty selection?
     * @return true if some text is selected
     */
    virtual bool selection () const = 0;

    /**
     * Retrieve the selected text
     * @return the selected text
     */
    virtual QString selectionText () const = 0;

    /**
     * Remove the current Selection (not Text)
     * @return success
     */
    virtual bool removeSelection () = 0;

    /**
     * Remove the current Selection together with the selected text!
     * @return success
     */
    virtual bool removeSelectionText () = 0;

    /**
     * Retrieve selection start position
     * @return selection start
     */
    virtual const Cursor &selectionStart () const = 0;

    /**
     * Retrieve selection end position
     * @return selection end
     */
    virtual const Cursor &selectionEnd () const = 0;

  /**
   * Blockselection stuff
   */
  public:
   /**
    * Set block selection mode to state "on"
    * @param on should block selection be active?
    * @return success
    */
    virtual bool setBlockSelection (bool on) = 0;

   /**
    * Returns the status of the selection mode - true indicates block selection mode is on.
    * If this is true, selections applied via the SelectionInterface are handled as
    * blockselections and the copy'n'paste functions works on
    * rectangular blocks of text rather than normal.
    * @return is block selection enabled?
    */
    virtual bool blockSelection () const = 0;

  /**
   * SIGNALS
   * following signals should be emitted by the editor view
   * if the selection state changes both on selection change itself and on change
   * of blockselection mode!
   * @param view view in which the selection has changed
   */
  private:
    virtual void selectionChanged (View *view) = 0;

  private:
    /**
     * Private d-pointer
     */
    class PrivateView *m_d;

    /**
     * view number
     */
    int m_viewNumber;
};

}

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
