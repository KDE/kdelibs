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

#ifndef _KATE_DOCUMENT_INCLUDE_
#define _KATE_DOCUMENT_INCLUDE_

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <kaction.h>

/**
 * Kate namespace
 * All classes in this namespace must stay BC
 * during one major release series (e.g. 3.x, 4.x, ...)
 */
namespace Kate
{

/** This interface provides access to the Kate Document class.
*/
class KATEPARTINTERFACES_EXPORT Document : public KTextEditor::Document
{
  Q_OBJECT

  public:
    Document ();
    Document ( QObject* parent, const char* name );
    virtual ~Document ();

  public:
    /**
     * Returns the document name.
     */
    virtual QString docName () { return 0L; };

    /**
     * Reloads the current document from disk if possible
     */
    virtual void reloadFile() = 0;

  /*
   * there static methodes are usefull to turn on/off the dialogs
   * kate part shows up on open file errors + file changed warnings
   * open file errors default on, file changed warnings default off, better
   * for other apps beside kate app using the part
   */
  public:
    // default true
    static void setOpenErrorDialogsActivated (bool on);

    // default false
    static void setFileChangedDialogsActivated (bool on);

    static const QString &defaultEncoding ();

  protected:
    static bool s_openErrorDialogsActivated;
    static bool s_fileChangedDialogsActivated;

    static QString s_defaultEncoding;
};

/**
 * Check if given document is a Kate::Document
 * @param doc KTextEditor document
 * @return 0 if no success, else the Kate::Document
 */
KATEPARTINTERFACES_EXPORT Document *document (KTextEditor::Document *doc);

}

#endif
