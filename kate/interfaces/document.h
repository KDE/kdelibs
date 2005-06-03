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
#include <ktexteditor/editinterface.h>
#include <ktexteditor/undointerface.h>
#include <ktexteditor/cursorinterface.h>
#include <ktexteditor/documentinfo.h>
#include <ktexteditor/selectioninterface.h>
#include <ktexteditor/selectioninterfaceext.h>
#include <ktexteditor/blockselectioninterface.h>
#include <ktexteditor/searchinterface.h>
#include <ktexteditor/highlightinginterface.h>
#include <ktexteditor/configinterface.h>
#include <ktexteditor/configinterfaceextension.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/markinterfaceextension.h>
#include <ktexteditor/wordwrapinterface.h>
#include <ktexteditor/printinterface.h>
#include <ktexteditor/variableinterface.h>
#include <ktexteditor/encodinginterface.h>
#include <ktexteditor/modificationinterface.h>

#include <kaction.h>

class KCompletion;

/**
 * Kate namespace
 * All classes in this namespace must stay BC
 * during one major release series (e.g. 3.x, 4.x, ...)
 */
namespace Kate
{

class View;

/**
 * Kate Commands
 */
class KATEPARTINTERFACES_EXPORT Command
{
  public:
    Command () {};
    virtual ~Command () {};

  public:
    /**
     * Pure text start part of the commands which can be handled by this object
     * which means i.e. for s/sdl/sdf/g => s or for char:1212 => char
     */
    virtual QStringList cmds () = 0;

    /**
     * Execute this command for the given view and cmd string, return a bool
     * about success, msg for status
     */
    virtual bool exec (View *view, const QString &cmd, QString &msg) = 0;

    /**
     * Shows help for the given view and cmd string, return a bool
     * about success, msg for status
     */
    virtual bool help (View *view, const QString &cmd, QString &msg) = 0;
};

/**
 * Extension to the Command interface, allowing to interact with commands
 * during typing. This allows for completion and for example the isearch
 * plugin. If you develop a command that wants to complete or process text
 * as thu user types the arguments, or that has flags, you can have
 * your command inherit this class.
 */
class CommandExtension
{
  public:
    CommandExtension() {;}
    virtual ~CommandExtension() {;}

    /**
     * Fill in a list of flags to complete from. Each flag is a single letter,
     * any following text in the string is taken to be a description of the
     * flag's meaning, and showed to the user as a hint.
     * Implement this method if your command has flags.
     *
     * This method is called each time the flag string in the typed command
     * is changed, so that the available flags can be adjusted. When completions
     * are displayed, existing flags are left out.
     *
     */ //### this is yet to be tried
    virtual void flagCompletions( QStringList& /*list*/ ) {;}

    /**
     * @return a KCompletion object that will substitute the command line default
     * one while typing the first argument to the command. The text will be
     * added to the command seperated by one space character.
     *
     * Implement this method if your command can provide a completion object.
     *
     * @param cmdname The command name associated with this request.
     */
    virtual KCompletion *completionObject( const QString & cmdname, Kate::View * /*view*/ ) { Q_UNUSED(cmdname); return 0L; }

    /**
     * @return whether this command wants to process text interactively given the @p cmdname.
     * If true, the command's processText() method is called when the
     * text in the command line is changed.
     *
     * Reimplement this to return true, if your commands wants to process the
     * text as typed.
     *
     * @param cmdname the command name associated with this query.
     */
    virtual bool wantsToProcessText( const QString &cmdname ) { Q_UNUSED(cmdname); return false; }

    /**
     * This is called by the commandline each time the argument text for the
     * command changes, if wantsToProcessText() returns true.
     * @param view The current view
     * @param text The current command text typed by the user.
     */ // ### yet to be tested. The obvious candidate is isearch.
    virtual void processText( Kate::View *view, const QString &text ) { Q_UNUSED(view); Q_UNUSED(text); }
};

/** This interface provides access to the Kate Document class.
*/
class KATEPARTINTERFACES_EXPORT Document : public KTextEditor::Document, public KTextEditor::EditInterface,
                     public KTextEditor::UndoInterface, public KTextEditor::CursorInterface,
                     public KTextEditor::SelectionInterface, public KTextEditor::SearchInterface,
                     public KTextEditor::HighlightingInterface, public KTextEditor::BlockSelectionInterface,
                     public KTextEditor::ConfigInterface, public KTextEditor::MarkInterface,
                     public KTextEditor::PrintInterface, public KTextEditor::WordWrapInterface,
                     public KTextEditor::MarkInterfaceExtension,
                     public KTextEditor::EncodingInterface,
                     public KTextEditor::SelectionInterfaceExt,
                     public KTextEditor::DocumentInfoInterface,
                     public KTextEditor::VariableInterface,
                     public KTextEditor::ModificationInterface
{
  Q_OBJECT

  public:
    Document ();
    Document ( QObject* parent, const char* name );
    virtual ~Document ();

  /**
   * Commands handling
   */
  public:
    static bool registerCommand (Command *cmd);
    static bool unregisterCommand (Command *cmd);
    static Command *queryCommand (const QString &cmd);

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

/**
 * Creates a new Kate::Document object
 */
KATEPARTINTERFACES_EXPORT Document *createDocument ( QObject *parent = 0, const char *name = 0 );

}

#endif
