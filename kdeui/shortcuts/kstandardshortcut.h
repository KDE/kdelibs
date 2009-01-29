/* This file is part of the KDE libraries
    Copyright (C) 1997 Stefan Taferner (taferner@kde.org)
    Copyright (C) 2000 Nicolas Hadacek (hadacek@kde.org)
    Copyright (C) 2001,2002 Ellis Whitehead (ellis@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KSTANDARDSHORTCUT_H
#define KSTANDARDSHORTCUT_H

#include <QtCore/QString>

#include "kshortcut.h"


/**
 * \namespace KStandardShortcut
 * Convenient methods for access to the common accelerator keys in
 * the key configuration. These are the standard keybindings that should
 * be used in all KDE applications. They will be configurable,
 * so do not hardcode the default behavior.
 */
namespace KStandardShortcut
{ // STUFF WILL BREAK IF YOU DON'T READ THIS!!!
  /*
   *Always add new std-accels to the end of this enum, never in the middle!
   *Don't forget to add the corresponding entries in g_infoStandardShortcut[] in kstandardshortcut.cpp, too.
   *Values of elements here and positions of the corresponding entries in
   *the big array g_infoStandardShortcut[] ABSOLUTELY MUST BE THE SAME.
   * !!!    !!!!   !!!!!    !!!!
   *    !!!!    !!!     !!!!    !!!!
   * Remember to also update kdoctools/genshortcutents.cpp.
   *
   * Other Rules:
   *
   * - Never change the name of an existing shortcut
   * - Never translate the name of an shortcut
   */

  /**
   * Defines the identifier of all standard accelerators.
   */
  enum StandardShortcut {
    //C++ requires that the value of an enum symbol be one more than the previous one.
	//This means that everything will be well-ordered from here on.
    AccelNone=0,
    // File menu
    Open, New, Close, Save,
    // The Print item
    Print,
    Quit,
    // Edit menu
    Undo, Redo, Cut, Copy, Paste,     PasteSelection,
    SelectAll, Deselect, DeleteWordBack, DeleteWordForward,
    Find, FindNext, FindPrev, Replace,
    // Navigation
    Home, Begin, End, Prior, Next,
    Up, Back, Forward, Reload,
    // Text Navigation
    BeginningOfLine, EndOfLine, GotoLine,
    BackwardWord, ForwardWord,
    // View parameters
    AddBookmark, ZoomIn, ZoomOut, FullScreen,
    ShowMenubar,
    // Tabular navigation
    TabNext, TabPrev,
    // Help menu
    Help, WhatsThis,
    // Text completion
    TextCompletion, PrevCompletion, NextCompletion, SubstringCompletion,

    RotateUp, RotateDown,

    OpenRecent,
    SaveAs,
    Revert,
    PrintPreview,
    Mail,
    Clear,
    ActualSize,
    FitToPage,
    FitToWidth,
    FitToHeight,
    Zoom,
    Goto,
    GotoPage,
    DocumentBack,
    DocumentForward,
    EditBookmarks,
    Spelling,
    ShowToolbar,
    ShowStatusbar,
    SaveOptions,
    KeyBindings,
    Preferences,
    ConfigureToolbars,
    ConfigureNotifications,
    TipofDay,
    ReportBug,
    SwitchApplicationLanguage,
    AboutApp,
    AboutKDE,

    // Insert new items here!

    StandardShortcutCount // number of standard shortcuts
  };

  /**
   * Returns the keybinding for @p accel.
   * On X11, if QApplication was initialized with GUI disabled, the
   * default keybinding will always be returned.
   * @param id the id of the accelerator
   */
  KDEUI_EXPORT const KShortcut &shortcut(StandardShortcut id);

  /**
   * Returns a unique name for the given accel.
   * @param id the id of the accelerator
   * @return the unique name of the accelerator
   */
  KDEUI_EXPORT QString name(StandardShortcut id);

  /**
   * Returns a localized label for user-visible display.
   * @param id the id of the accelerator
   * @return a localized label for the accelerator
   */
  KDEUI_EXPORT QString label(StandardShortcut id);

  /**
   * Returns an extended WhatsThis description for the given accelerator.
   * @param id the id of the accelerator
   * @return a localized description of the accelerator
   */
  KDEUI_EXPORT QString whatsThis(StandardShortcut id);

  /**
   * Return the StandardShortcut id of the standard accel action which
   * uses this key sequence, or AccelNone if none of them do.
   * This is used by class KKeyChooser.
   * @param keySeq the key sequence to search
   * @return the id of the standard accelerator, or AccelNone if there
   *          is none
   */
  KDEUI_EXPORT StandardShortcut find(const QKeySequence &keySeq);

  /**
   * Return the StandardShortcut id of the standard accel action which
   * has \a keyName as its name, or AccelNone if none of them do.
   * This is used by class KKeyChooser.
   * @param keyName the key sequence to search
   * @return the id of the standard accelerator, or AccelNone if there
   *          is none
   */
  KDEUI_EXPORT StandardShortcut find(const char *keyName);

  /**
   * Returns the hardcoded default shortcut for @p id.
   * This does not take into account the user's configuration.
   * @param id the id of the accelerator
   * @return the default shortcut of the accelerator
   */
  KDEUI_EXPORT KShortcut hardcodedDefaultShortcut(StandardShortcut id);

  /**
   * Saves the new shortcut \a cut for standard accel \a id.
   */
  KDEUI_EXPORT void saveShortcut(StandardShortcut id, const KShortcut &newShortcut);

  /**
   * Open file. Default: Ctrl-o
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &open();

  /**
   * Create a new document (or whatever). Default: Ctrl-n
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &openNew();

  /**
   * Close current document. Default: Ctrl-w
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &close();

  /**
   * Save current document. Default: Ctrl-s
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &save();

  /**
   * Print current document. Default: Ctrl-p
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &print();

  /**
   * Quit the program. Default: Ctrl-q
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &quit();

  /**
   * Undo last operation. Default: Ctrl-z
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &undo();

  /**
   * Redo. Default: Shift-Ctrl-z
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &redo();

  /**
   * Cut selected area and store it in the clipboard. Default: Ctrl-x
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &cut();

  /**
   * Copy selected area into the clipboard. Default: Ctrl-c
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &copy();

  /**
   * Paste contents of clipboard at mouse/cursor position. Default: Ctrl-v
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &paste();

  /**
   * Paste the selection at mouse/cursor position. Default: Ctrl-Shift-Insert
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &pasteSelection();

  /**
   * Select all. Default: Ctrl-A
   * @return the shortcut of the standard accelerator
   **/
  KDEUI_EXPORT const KShortcut &selectAll();

  /**
   * Delete a word back from mouse/cursor position. Default: Ctrl-Backspace
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &deleteWordBack();

  /**
   * Delete a word forward from mouse/cursor position. Default: Ctrl-Delete
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &deleteWordForward();

  /**
   * Find, search. Default: Ctrl-f
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &find();

  /**
   * Find/search next. Default: F3
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &findNext();

  /**
   * Find/search previous. Default: Shift-F3
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &findPrev();

  /**
   * Find and replace matches. Default: Ctrl-r
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &replace();

  /**
   * Zoom in. Default: Ctrl-Plus
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &zoomIn();

  /**
   * Zoom out. Default: Ctrl-Minus
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &zoomOut();

  /**
   * Toggle insert/overwrite (with visual feedback, e.g. in the statusbar). Default: Insert
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &insert();

  /**
   * Goto home page. Default: Alt-Home
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &home();

  /**
   * Goto beginning of the document. Default: Ctrl-Home
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &begin();

  /**
   * Goto end of the document. Default: Ctrl-End
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &end();

  /**
   * Goto beginning of current line. Default: Home
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &beginningOfLine();

  /**
   * Goto end of current line. Default: End
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &endOfLine();

  /**
   * Scroll up one page. Default: Prior
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &prior();

  /**
   * Scroll down one page. Default: Next
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &next();

  /**
   * Go to line. Default: Ctrl+G
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &gotoLine();

  /**
   * Add current page to bookmarks. Default: Ctrl+B
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &addBookmark();

  /**
   * Next Tab. Default: Ctrl-<
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &tabNext();

  /**
   * Previous Tab. Default: Ctrl->
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &tabPrev();

  /**
   * Full Screen Mode. Default: Ctrl+Shift+F
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &fullScreen();

  /**
   * Help the user in the current situation. Default: F1
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &help();

  /**
   * Complete text in input widgets. Default Ctrl+E
   * @return the shortcut of the standard accelerator
   **/
  KDEUI_EXPORT const KShortcut &completion();

  /**
   * Iterate through a list when completion returns
   * multiple items. Default: Ctrl+Up
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &prevCompletion();

  /**
   * Iterate through a list when completion returns
   * multiple items. Default: Ctrl+Down
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &nextCompletion();

  /**
   * Find a string within another string or list of strings.
   * Default: Ctrl-T
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &substringCompletion();

  /**
   * Help users iterate through a list of entries. Default: Up
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &rotateUp();

  /**
   * Help users iterate through a list of entries. Default: Down
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &rotateDown();

  /**
   * What's This button. Default: Shift+F1
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &whatsThis();

  /**
   * Reload. Default: F5
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &reload();

  /**
   * Up. Default: Alt+Up
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &up();

  /**
   * Back. Default: Alt+Left
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &back();

  /**
   * Forward. Default: ALT+Right
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &forward();

  /**
   * BackwardWord. Default: Ctrl+Left
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &backwardWord();

  /**
   * ForwardWord. Default: Ctrl+Right
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &forwardWord();

  /**
   * Show Menu Bar.  Default: Ctrl-M
   * @return the shortcut of the standard accelerator
   */
  KDEUI_EXPORT const KShortcut &showMenubar();

}

#endif // KSTANDARDSHORTCUT_H
