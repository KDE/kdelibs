/* This file is part of the KDE libraries
   Copyright (C) 2005 Christoph Cullmann <cullmann@kde.org>

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

#ifndef __ktexteditor_editor_h__
#define __ktexteditor_editor_h__

// our main baseclass of the KTextEditor::Editor
#include <QObject>

class KAboutData;

namespace KTextEditor
{

class Document;

/**
 * This class represents the Editor of the editor
 * Each KTextEditor part must reimplement this Editor,
 * to allow easy access to the editor object
 */
class KTEXTEDITOR_EXPORT Editor : public QObject
{
  Q_OBJECT

  public:
    /**
     * Editor Constructor
     * @param parent parent object
     */
    Editor ( QObject *parent );

    /**
     * virtual destructor
     */
    virtual ~Editor ();

  /**
   * Stuff to create and manage the documents
   */
  public:
    /**
     * Create a new document object
     * @param parent parent object
     * @return created KTextEditor::Document
     */
    virtual Document *createDocument ( QObject *parent ) = 0;

    /**
     * Returns a list of all documents of this editor.
     * @return list of all existing documents
     */
    virtual const QList<Document*> &documents () = 0;

    /**
     * return the about data
     */
    virtual const KAboutData* aboutData() const =0;
  signals:
    /**
     * emit this after successfull document creation in createDocument
     * @param editor editor which created the new document
     * @param document the newly created document instance
     */
    void documentCreated (Editor *editor, Document *document);
};

KTEXTEDITOR_EXPORT Editor *editor ( const char *libname );

}

#endif
