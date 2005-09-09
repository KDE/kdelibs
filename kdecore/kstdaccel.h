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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KSTDACCEL_H
#define KSTDACCEL_H

#include <qstring.h>
#include <kshortcut.h>
#include "kdelibs_export.h"

class QKeyEvent;

/**
 * \namespace KStdAccel
 * Convenient methods for access to the common accelerator keys in
 * the key configuration. These are the standard keybindings that should
 * be used in all KDE applications. They will be configurable,
 * so do not hardcode the default behavior.
 *
 * If you want real configurable keybindings in your applications,
 * please checkout the class KAccel in kaccel.h
 * @see KAccelShortcutList
 */
namespace KStdAccel
{
  // Always add new std-accels to the end of this enum, never in the middle!
  /**
   * Defines the identifier of all standard accelerators.
   */
  enum StdAccel {
    AccelNone,
    // File menu
    Open, New, Close, Save,
    // The Print item
    Print,
    Quit,
    // Edit menu
    Undo, Redo, Cut, Copy, Paste, SelectAll, Deselect, DeleteWordBack,
    DeleteWordForward, Find, FindNext, FindPrev, Replace,
    // Navigation
    Home, End, Prior, Next, GotoLine, AddBookmark, ZoomIn, ZoomOut,
    Up, Back, Forward, Reload, PopupMenuContext, ShowMenubar,
    // Help menu
    Help, WhatsThis,
    // Text completion
    TextCompletion, PrevCompletion, NextCompletion, SubstringCompletion,
    RotateUp, RotateDown,

    // Tabular navigation
    TabNext,           ///< @since 3.2
    TabPrev,           ///< @since 3.2

    // Full screen mode
    FullScreen,        ///< @since 3.2

    // Text Navigation
    BackwardWord,      ///< @since 3.3
    ForwardWord,       ///< @since 3.3
    BeginningOfLine,   ///< @since 3.3
    EndOfLine,         ///< @since 3.3

    PasteSelection     ///< @since 3.4

#ifndef KDE_NO_COMPAT
    , WhatThis = WhatsThis
#endif
  };

  /**
   * Returns the keybinding for @p accel.
   * @param id the id of the accelerator
   */
  KDECORE_EXPORT const KShortcut& shortcut(StdAccel id);

  /**
   * Returns a unique name for the given accel.
   * @param id the id of the accelerator
   * @return the unique name of the accelerator
   */
  KDECORE_EXPORT QString name(StdAccel id);

  /**
   * Returns a localized label for user-visible display.
   * @param id the id of the accelerator
   * @return a localized label for the accelerator
   */
  KDECORE_EXPORT QString label(StdAccel id);

  /**
   * Returns an extended WhatsThis description for the given accelerator.
   * @param id the id of the accelerator
   * @return a localized description of the accelerator
   */
  KDECORE_EXPORT QString whatsThis(StdAccel id);

  /**
   * Return the StdAccel id of the standard accel action which
   * uses this key sequence, or AccelNone if none of them do.
   * This is used by class KKeyChooser.
   * @param keySeq the key sequence to search
   * @return the id of the standard accelerator, or AccelNone if there
   *          is none
   */
  KDECORE_EXPORT StdAccel findStdAccel( const KKeySequence &keySeq );

  /**
   * Returns the hardcoded default shortcut for @p id.
   * This does not take into account the user's configuration.
   * @param id the id of the accelerator
   * @return the default shortcut of the accelerator
   */
  KDECORE_EXPORT KShortcut shortcutDefault(StdAccel id);
  /**
   * Returns the hardcoded default 3 modifier shortcut for @p id.
   * This does not take into account the user's configuration.
   * @param id the id of the accelerator
   * @return the default 3 modifier shortcut
   */
  KDECORE_EXPORT KShortcut shortcutDefault3(StdAccel id);
  /**
   * Returns the hardcoded default 4 modifier shortcut for @p id.
   * This does not take into account the user's configuration.
   * @param id the id of the accelerator
   * @return the default 4 modifier shortcut
   */
  KDECORE_EXPORT KShortcut shortcutDefault4(StdAccel id);

  /**
   * Open file. Default: Ctrl-o
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& open();

  /**
   * Create a new document (or whatever). Default: Ctrl-n
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& openNew();

  /**
   * Close current document. Default: Ctrl-w
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& close();

  /**
   * Save current document. Default: Ctrl-s
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& save();

  /**
   * Print current document. Default: Ctrl-p
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& print();

  /**
   * Quit the program. Default: Ctrl-q
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& quit();

  /**
   * Undo last operation. Default: Ctrl-z
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& undo();

  /**
   * Redo. Default: Shift-Ctrl-z
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& redo();

  /**
   * Cut selected area and store it in the clipboard. Default: Ctrl-x
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& cut();

  /**
   * Copy selected area into the clipboard. Default: Ctrl-c
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& copy();

  /**
   * Paste contents of clipboard at mouse/cursor position. Default: Ctrl-v
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& paste();

  /**
   * Paste the selection at mouse/cursor position. Default: Ctrl-Shift-Insert
   * @return the shortcut of the standard accelerator
   * @since 3.4
   */
  KDECORE_EXPORT const KShortcut& pasteSelection();

  /**
   * Reload. Default: Ctrl-A
   * @return the shortcut of the standard accelerator
   **/
  KDECORE_EXPORT const KShortcut& selectAll();

  /**
   * Delete a word back from mouse/cursor position. Default: Ctrl-Backspace
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& deleteWordBack();

  /**
   * Delete a word forward from mouse/cursor position. Default: Ctrl-Delete
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& deleteWordForward();

  /**
   * Find, search. Default: Ctrl-f
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& find();

  /**
   * Find/search next. Default: F3
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& findNext();

  /**
   * Find/search previous. Default: Shift-F3
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& findPrev();

  /**
   * Find and replace matches. Default: Ctrl-r
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& replace();

  /**
   * Zoom in. Default: Ctrl-Plus
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& zoomIn();

  /**
   * Zoom out. Default: Ctrl-Minus
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& zoomOut();

  /**
   * Toggle insert/overwrite (with visual feedback, e.g. in the statusbar). Default: Insert
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& insert();

  /**
   * Goto beginning of the document. Default: Ctrl-Home
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& home();

  /**
   * Goto end of the document. Default: Ctrl-End
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& end();

  /**
   * Goto beginning of current line. Default: Home
   * @return the shortcut of the standard accelerator
   * @since 3.3
   */
  KDECORE_EXPORT const KShortcut& beginningOfLine();

  /**
   * Goto end of current line. Default: End
   * @return the shortcut of the standard accelerator
   * @since 3.3
   */
  KDECORE_EXPORT const KShortcut& endOfLine();

  /**
   * Scroll up one page. Default: Prior
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& prior();

  /**
   * Scroll down one page. Default: Next
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& next();

  /**
   * Go to line. Default: Ctrl+G
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& gotoLine();

  /**
   * Add current page to bookmarks. Default: Ctrl+B
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& addBookmark();

  /**
   * Next Tab. Default: Ctrl-<
   * @return the shortcut of the standard accelerator
   * @since 3.2
   */
  KDECORE_EXPORT const KShortcut& tabNext();

  /**
   * Previous Tab. Default: Ctrl->
   * @return the shortcut of the standard accelerator
   * @since 3.2
   */
  KDECORE_EXPORT const KShortcut& tabPrev();

  /**
   * Full Screen Mode. Default: Ctrl+Shift+F
   * @return the shortcut of the standard accelerator
   * @since 3.2
   */
  KDECORE_EXPORT const KShortcut& fullScreen();

  /**
   * Help the user in the current situation. Default: F1
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& help();

  /**
   * Complete text in input widgets. Default Ctrl+E
   * @return the shortcut of the standard accelerator
   **/
  KDECORE_EXPORT const KShortcut& completion();

  /**
   * Iterate through a list when completion returns
   * multiple items. Default: Ctrl+Up
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& prevCompletion();

  /**
   * Iterate through a list when completion returns
   * multiple items. Default: Ctrl+Down
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& nextCompletion();

  /**
   * Find a string within another string or list of strings.
   * Default: Ctrl-T
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& substringCompletion();

  /**
   * Help users iterate through a list of entries. Default: Up
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& rotateUp();

  /**
   * Help users iterate through a list of entries. Default: Down
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& rotateDown();

  /**
   * popup a context menu. Default: Menu
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& popupMenuContext();

  /**
   * What's This button. Default: Shift+F1
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& whatsThis();

  /**
   * Reload. Default: F5
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& reload();

  /**
   * Up. Default: Alt+Up
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& up();

  /**
   * Back. Default: Alt+Left
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& back();

  /**
   * Forward. Default: ALT+Right
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& forward();

  /**
   * BackwardWord. Default: Ctrl+Left
   * @return the shortcut of the standard accelerator
   * @since 3.3
   */
  KDECORE_EXPORT const KShortcut& backwardWord();

  /**
   * ForwardWord. Default: Ctrl+Right
   * @return the shortcut of the standard accelerator
   * @since 3.3
   */
  KDECORE_EXPORT const KShortcut& forwardWord();

  /**
   * Show Menu Bar.  Default: Ctrl-M
   * @return the shortcut of the standard accelerator
   */
  KDECORE_EXPORT const KShortcut& showMenubar();

}

#endif
