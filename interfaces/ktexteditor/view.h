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

#include <qwidget.h>
#include <kxmlguiclient.h>

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
    View ( Document *document, QWidget *parent );

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
     * Access the parent Document.
     * @return document
     */
    Document *document ();

  private:
    /**
     * Private d-pointer
     */
    class PrivateView *m_d;

    /**
     * view number
     */
    int m_viewNumber;

    /**
     * parent document
     */
    Document *m_doc;
};

}

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
