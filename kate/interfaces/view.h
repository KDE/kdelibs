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

#ifndef _KATE_VIEW_INCLUDE_
#define _KATE_VIEW_INCLUDE_

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/clipboardinterface.h>
#include <ktexteditor/popupmenuinterface.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/codecompletioninterface.h>
#include <ktexteditor/dynwordwrapinterface.h>
#include <ktexteditor/sessionconfiginterface.h>

class KConfig;

namespace Kate
{

class Document;

/**
  The Kate::View text editor interface.
  @author Cullmann Christoph, modified by rokrau (6/21/01)
*/
class KATEPARTINTERFACES_EXPORT View : public KTextEditor::View, public KTextEditor::ClipboardInterface,
              public KTextEditor::PopupMenuInterface, public KTextEditor::ViewCursorInterface,
              public KTextEditor::CodeCompletionInterface, 
              public KTextEditor::SessionConfigInterface,
              public KTextEditor::DynWordWrapInterface
{
  Q_OBJECT

  public:
    /**
     Constructor (should much rather take a reference to the document).
    */
    View ( KTextEditor::Document *, QWidget *, const char *name = 0 );
    /**
     Destructor, you need a destructor if Scott Meyers says so.
    */
    virtual ~View ();
    /**
     Set editor mode
    */
    virtual bool isOverwriteMode() const  { return false; }

    /**
     Return values for "save" related commands.
    */
    enum saveResult { SAVE_OK, SAVE_CANCEL, SAVE_RETRY, SAVE_ERROR };

  public slots:
    /**
      Saves the file under the current file name. If the current file
      name is Untitled, as it is after a call to newFile(), this routine will
      call saveAs().
    */
    virtual saveResult save() { return SAVE_CANCEL; };
    /**
      Allows the user to save the file under a new name.
    */
    virtual saveResult saveAs() { return SAVE_CANCEL; };

  public:
    /**
     Returns a pointer to the document of the view.
    */
    virtual Document *getDoc () { return 0L; }

  signals:
    void gotFocus (View *);
//  void newStatus(); // Kate app connects to this signal, should be in the interface

  public:
    virtual void setActive (bool b) = 0;
    virtual bool isActive () = 0;
};

KATEPARTINTERFACES_EXPORT View *view (KTextEditor::View *view);

}

#endif
