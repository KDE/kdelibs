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

#ifndef __ktexteditor_document_h__
#define __ktexteditor_document_h__

// the very important KTextEditor::Cursor class
#include <ktexteditor/cursor.h>

// our main baseclass of the KTextEditor::Document
#include <kparts/part.h>

// the list of views
#include <QList>

/**
 * Namespace for the KDE Text Editor Interfaces
 */
namespace KTextEditor
{

class View;

/**
 * The main class representing a text document.
 * This class provides access to the document's views.
 */
class KTEXTEDITOR_EXPORT Document : public KParts::ReadWritePart
{
  Q_OBJECT

  public:
    /**
     * Document Constructor
     * @param parent parent object
     * @param name name
     */
    Document ( QObject *parent = 0, const char *name = 0 );

    /**
     * virtual destructor
     */
    virtual ~Document ();

    /**
     * Returns the global number of this document in your app.
     * @return document number
     */
    int documentNumber () const;

  /**
   * Stuff to create and manage the views of this document
   */
  public:
    /**
     * Create a view that will display the document data. You can create as many
     * views as you like. When the user modifies data in one view then all other
     * views will be updated as well.
     * @param parent parent widget
     * @return created KTextEditor::View
     */
    virtual View *createView ( QWidget *parent ) = 0;

    /**
     * Returns a list of all views of this document.
     * @return list of all existing views
     */
    virtual const QList<View*> &views () const = 0;

  /**
   * General information about this document and it's content
   */
  public:
    /**
     * Returns this document's name
     * The editor part should provide some meaningful name, like some unique
     * Untitled XYZ for document without url or basename for documents with
     * url
     * @return readable document name
     */
    virtual const QString &documentName () const = 0;

    /**
     * Returns this document's mimetype
     * @return mimetype
     */
    virtual QString mimeType() = 0;

 /**
  * Methodes to create/end editing sequences
  */
 public:
    /**
     * Begin an editing sequence.  Edit commands during this sequence will be
     * bunched together such that they represent a single undo command in the
     * editor, and so that repaint events do not occur inbetween.
     *
     * Your application should not return control to the event loop while it
     * has an unterminated (no matching editEnd() call) editing sequence
     * (result undefined) - so do all of your work in one go...
     */
    virtual void editBegin () = 0;

	  /**
	   * End an editing sequence.
	   */
    virtual void editEnd () = 0;

  /**
   * General access to the document's text content
   */
  public:
    /**
     * retrieve the document content
     * @return the complete document content
     */
    virtual QString text () const = 0;

    /**
     * retrieve part of the document content
     * @param startPosition start position of text to retrieve
     * @param endPosition end position of text to retrieve
     * @return the requested text part, "" for invalid areas
     */
    virtual QString text ( const Cursor &startPosition, const Cursor &endPosition ) const = 0;

    /**
     * retrieve a single text line
     * @param line the wanted line
     * @return the requested line, "" for invalid line numbers
     */
    virtual QString line ( int line ) const = 0;

    /**
     * count of lines in document
     * @return The current number of lines in the document
     */
    virtual int lines () const = 0;

    /**
     * count of characters in document
     * @return the number of characters in the document
     */
    virtual int length () const = 0;

    /**
     * retrieve the length of a given line in characters
     * @param line line to get length from
     * @return the number of characters in the line (-1 if no line "line")
     */
    virtual int lineLength ( int line ) const = 0;

    /**
     * Set the given text as new document content
     * @param text new content for the document
     * @return success
     */
    virtual bool setText ( const QString &text ) = 0;

    /**
     * Removes the whole content of the document
     * @return success
     */
    virtual bool clear () = 0;

    /**
     * Inserts text at given position
     * @param position position to insert the text
     * @param text text to insert
     * @return success
     */
    virtual bool insertText ( const Cursor &position, const QString &text ) = 0;

    /**
     * remove part of the document content
     * @param startPosition start position of text to remove
     * @param endPosition end position of text to remove
     * @return success
     */
    virtual bool removeText ( const Cursor &startPosition, const Cursor &endPosition ) = 0;

    /**
     * Insert line(s) at the given line number.
     * Use insertLine(numLines(), text) to append line at end of document
     * @param line line where to insert
     * @param text text which should be inserted
     * @return success
     */
    virtual bool insertLine ( int line, const QString &text ) = 0;

    /**
     * Remove line at the given line number.
     * @param line line to remove
     * @return success
     */
    virtual bool removeLine ( int line ) = 0;

  /**
   * SIGNALS
   * following signals should be emitted by the document
   * if the text content is changed
   */
	private:
	  /**
	   * Text changed!
	   * @param document document which emited this signal
	   */
	  virtual void textChanged (Document *document) = 0;

  private:
    /**
     * Private d-pointer
     */
    class PrivateDocument *m_d;

    /**
     * document number
     */
    int m_documentNumber;
};

KTEXTEDITOR_EXPORT Document *createDocument ( const char* libname, QObject *parent );

}

#endif
