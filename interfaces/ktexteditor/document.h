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

#include <kparts/part.h>

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
