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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KSTDACCEL_H
#define KSTDACCEL_H

#include <qstring.h>
#include <kshortcut.h>

class QKeyEvent;
class KAccelActions;

/**
 * Convenient methods for access to the common accelerator keys in
 * the key configuration. These are the standard keybindings that should
 * be used in all KDE applications. They will be configurable,
 * so do not hardcode the default behavior.
 *
 * If you want real configurable keybindings in your applications,
 * please checkout the class @ref KAccel in kaccel.h
 */
namespace KStdAccel
{
  // Always add new std-accels to the end of this enum, never in the middle!
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
    RotateUp, RotateDown

#ifndef KDE_NO_COMPAT
    , WhatThis = WhatsThis
#endif
  };

  /**
   * Returns the keybinding for @p accel.
   */
  const KShortcut& shortcut(StdAccel id);

  /**
   * Returns a unique name for the given @p accel.
   */
  QString name(StdAccel id);

  /**
   * Returns a localized label for user-visible display.
   */
  QString label(StdAccel id);

  /**
   * Returns an extended WhatsThis description for the given accelerator.
   */
  QString whatsThis(StdAccel id);

  /**
   * Return the StdAccel id of the standard accel action which
   * uses this key sequence, or AccelNone if none of them do.
   * This is used by class KKeyChooser.
   */
  StdAccel findStdAccel( const KKeySequence& );

  /**
   * Returns the hardcoded default shortcut for @p id.
   * This does not take into account the user's configuration.
   */
  KShortcut shortcutDefault(StdAccel id);
  /**
   * Returns the hardcoded default 3 modifier shortcut for @p id.
   * This does not take into account the user's configuration.
   */
  KShortcut shortcutDefault3(StdAccel id);
  /**
   * Returns the hardcoded default 4 modifier shortcut for @p id.
   * This does not take into account the user's configuration.
   */
  KShortcut shortcutDefault4(StdAccel id);

  /**
   * Open file. Default: Ctrl-o
   */
  const KShortcut& open();

  /**
   * Create a new document (or whatever). Default: Ctrl-n
   */
  const KShortcut& openNew();

  /**
   * Close current document. Default: Ctrl-w
   */
  const KShortcut& close();

  /**
   * Save current document. Default: Ctrl-s
   */
  const KShortcut& save();

  /**
   * Print current document. Default: Ctrl-p
   */
  const KShortcut& print();

  /**
   * Quit the program. Default: Ctrl-q
   */
  const KShortcut& quit();

  /**
   * Undo last operation. Default: Ctrl-z
   */
  const KShortcut& undo();

  /**
   * Redo. Default: Shift-Ctrl-z
   */
  const KShortcut& redo();

  /**
   * Cut selected area and store it in the clipboard. Default: Ctrl-x
   */
  const KShortcut& cut();

  /**
   * Copy selected area into the clipboard. Default: Ctrl-c
   */
  const KShortcut& copy();

  /**
   * Paste contents of clipboard at mouse/cursor position. Default: Ctrl-v
   */
  const KShortcut& paste();

  /**
   * Reload. Default: Ctrl-A
   **/
  const KShortcut& selectAll();

  /**
   * Delete a word back from mouse/cursor position. Default: Ctrl-Backspace
   */
  const KShortcut& deleteWordBack();

  /**
   * Delete a word forward from mouse/cursor position. Default: Ctrl-Delete
   */
  const KShortcut& deleteWordForward();

  /**
   * Find, search. Default: Ctrl-f
   */
  const KShortcut& find();

  /**
   * Find/search next. Default: F3
   */
  const KShortcut& findNext();

  /**
   * Find/search previous. Default: Shift-F3
   */
  const KShortcut& findPrev();

  /**
   * Find and replace matches. Default: Ctrl-r
   */
  const KShortcut& replace();

  /**
   * Zoom in. Default: Ctrl-Plus
   */
  const KShortcut& zoomIn();

  /**
   * Zoom out. Default: Ctrl-Minus
   */
  const KShortcut& zoomOut();

  /**
   * Toggle insert/overwrite (with visual feedback, e.g. in the statusbar). Default: Insert
   */
  const KShortcut& insert();

  /**
   * Goto beginning of current line. Default: Home
   */
  const KShortcut& home();

  /**
   * Goto end of current line. Default: End
   */
  const KShortcut& end();

  /**
   * Scroll up one page. Default: Prior
   */
  const KShortcut& prior();

  /**
   * Scroll down one page. Default: Next
   */
  const KShortcut& next();

  /**
   * Go to line. Default: Ctrl+G
   */
  const KShortcut& gotoLine();

  /**
   * Add current page to bookmarks. Default: Ctrl+B
   */
  const KShortcut& addBookmark();

  /**
   * Help the user in the current situation. Default: F1
   */
  const KShortcut& help();

  /**
   * Complete text in input widgets. Default Ctrl+E
   **/
  const KShortcut& completion();

  /**
   * Iterate through a list when completion returns
   * mutiple items. Default: Ctrl+Up
   */
  const KShortcut& prevCompletion();

  /**
   * Iterate through a list when completion returns
   * mutiple items. Default: Ctrl+Down
   */
  const KShortcut& nextCompletion();

  /**
   * Find a string within another string or list of strings.
   * Default: Ctrl-T
   */
  const KShortcut& substringCompletion();

  /**
   * Help users iterate through a list of entries. Default: Up
   */
  const KShortcut& rotateUp();

  /**
   * Help users iterate through a list of entries. Default: Down
   */
  const KShortcut& rotateDown();

  /**
   * popup a context menu. Default: Menu
   */
  const KShortcut& popupMenuContext();

  /**
   * What's This button. Default: Shift+F1
   */
  const KShortcut& whatsThis();

  /**
   * Reload. Default: F5
   */
  const KShortcut& reload();

  /**
   * Up. Default: Alt+Up
   */
  const KShortcut& up();

  /**
   * Back. Default: Alt+Left
   */
  const KShortcut& back();

  /**
   * Forward. Default: ALT+Right
   */
  const KShortcut& forward();

  /**
   * Show Menu Bar.  Default: Ctrl-M
   */
  const KShortcut& showMenubar();

#ifndef KDE_NO_COMPAT
  /**
   * Obsolete.  Use name().  Returns a string representation for @p accel.
   */
  QString action(StdAccel id);
  /**
   * Obsolete.  Use desc().  Returns a localized description of @p accel.
   */
  QString description(StdAccel id);
  /**
   * Obsolete.  Use shortcut().  Returns the keybinding for @p accel.
   */
  int key(StdAccel);
  /**
   * Obsolete.  Use shortcutDefault().
   */
  int defaultKey(StdAccel accel);

  /**
   * @deprecated.  Use KKey(const QKeyEvent*) == KKey(int).
   *
   * Compare the keys generated by the key event with
   * the value of the integer.
   *
   * If a modifier (Shift, Alt, Ctrl) key is present in
   * @ref QKeyEvent, its sum with the actual key value
   * is used for comparing it with the integer parameter.
   *
   * @param pEvent the key event to be used in the comparison.
   * @param keyQt the int value to be compared to the key event.
   *
   * @return true if the int value matches the integer representation of the @ref QKeyEvent
   */
  bool isEqual(const QKeyEvent* pEvent, int keyQt);
#endif // !KDE_NO_COMPAT

};

#endif
