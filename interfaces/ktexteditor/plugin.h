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

#ifndef __ktexteditor_plugin_h__
#define __ktexteditor_plugin_h__

#include <qobject.h>

#include <kdelibs_export.h>

namespace KTextEditor
{

class Document;
class View;

/**
 * Basic KTextEditor plugin class.
 * This plugin will be bound to a Document.
 */
class KTEXTEDITOR_EXPORT Plugin : public QObject
{
  Q_OBJECT

  public:
    /**
     * Plugin constructor
     * @param document parent document
     * @param name plugin name
     */
    Plugin ( Document *document = 0, const char *name = 0 );

    /**
     * virtual destructor
     */
    virtual ~Plugin ();

    /**
     * Returns the global number of this plugin in your app.
     * @return plugin number
     */
    int pluginNumber () const;

    /**
     * Access the parent Document.
     * @return document
     */
    Document *document ();

    /**
     * this method is called if the plugin gui should be added
     * to the given KTextEditor::View
     * @param view view to hang the gui in
     */
    virtual void addView (View *view) = 0;

    /**
     * this method is called if the plugin gui should be removed
     * from the given KTextEditor::View
     * @param view view to hang the gui out from
     */
    virtual void removeView (View *view) = 0;

  private:
    /**
     * Private d-pointer
     */
    class PrivatePlugin *m_d;

    /**
     * plugin number
     */
    int m_pluginNumber;

    /**
     * parent document
     */
    Document *m_doc;
};

KTEXTEDITOR_EXPORT Plugin *createPlugin ( const char* libname, Document *document );

}

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
