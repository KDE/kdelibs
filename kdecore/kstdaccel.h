/* This file is part of the KDE libraries
    Copyright (C) 1997 Stefan Taferner (taferner@kde.org)
    Copyright (C) 2000 Nicolas Hadacek (hadacek@kde.org)

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

class QKeyEvent;

/**
 * Convenient methods for access to the common accelerator keys in
 * the key configuration. These are the standard keybindings that should
 * be used in all KDE applications. They will be configurable,
 * so do not hardcode the default behavior.
 *
 * If you want real configurable keybindings in your applications,
 * please checkout the class @ref KAccel in kaccel.h
 */
class KStdAccel
{
public:
  KStdAccel() {}

  // Always add new std-accels to the end of this enum, never in the middle!
  enum StdAccel {
    Open=0, New, Close, Save, Print, Quit, Cut, Copy, Paste, Undo, Redo,
    Find, Replace, Insert, Home, End, Prior, Next, Help, FindNext, FindPrev,
    ZoomIn, ZoomOut, AddBookmark, TextCompletion, PrevCompletion, NextCompletion,
    RotateUp, RotateDown, PopupMenuContext, WhatThis, Reload, SelectAll, Up, Back,
    Forward, ShowMenubar, GotoLine, DeleteWordBack, DeleteWordForward, NB_STD_ACCELS
  };

  /** Open file. Default: Ctrl-o */
  static uint open();

  /** Create a new document (or whatever). Default: Ctrl-n */
  static uint openNew();

  /** Close current document. Default: Ctrl-w */
  static uint close();

  /** Save current document. Default: Ctrl-s */
  static uint save();

  /** Print current document. Default: Ctrl-p */
  static uint print();

  /** Quit the program. Default: Ctrl-q */
  static uint quit();

  /** Cut selected area and store it in the clipboard. Default: Ctrl-x */
  static uint cut();

  /** Copy selected area into the clipboard. Default: Ctrl-c */
  static uint copy();

  /** Paste contents of clipboard at mouse/cursor position. Default: Ctrl-v */
  static uint paste();

  /** Delete a word back from mouse/cursor position. Default: Ctrl-Backspace */
  static uint deleteWordBack();

  /** Delete a word forward from mouse/cursor position. Default: Ctrl-Delete */
  static uint deleteWordForward();

  /** Undo last operation. Default: Ctrl-z */
  static uint undo();

  /** Redo. Default: Shift-Ctrl-z */
  static uint redo();

  /** Find, search. Default: Ctrl-f */
  static uint find();

  /** Find/search next. Default: F3 */
  static uint findNext();

  /** Find/search previous. Default: Shift-F3 */
  static uint findPrev();

  /** Find and replace matches. Default: Ctrl-r */
  static uint replace();

  /** Zoom in. Default: Ctrl-Plus */
  static uint zoomIn();

  /** Zoom out. Default: Ctrl-Minus */
  static uint zoomOut();

  /** Toggle insert/overwrite (with visual feedback, e.g. in the statusbar). Default: Insert */
  static uint insert();

  /** Goto beginning of current line. Default: Home */
  static uint home();

  /** Goto end of current line. Default: End */
  static uint end();

  /** Scroll up one page. Default: Prior */
  static uint prior();

  /** Scroll down one page. Default: Next */
  static uint next();

  /** Go to line. Default: Ctrl+G */
  static uint gotoLine();

  /** Add current page to bookmarks. Default: Ctrl+B */
  static uint addBookmark();

  /** Help the user in the current situation. Default: F1 */
  static uint help();

  /** Complete text in input widgets. Default Ctrl+E **/
  static uint completion();

  /** Iterate through a list when completion returns
      mutiple items. Default: Ctrl+Up **/
  static uint prevCompletion();

  /** Iterate through a list when completion returns
      mutiple items. Default: Ctrl+Down **/
  static uint nextCompletion();

  /** Help users iterate through a list of enteries. Default: Up **/
  static uint rotateUp();

  /** Help users iterate through a list of enteries. Default: Down **/
  static uint rotateDown();

  /** popup a context menu. Default: Menu **/
  static uint popupMenuContext();

  /** What's This button. Default: Shift+F1 **/
  static uint whatsThis();

  /** Reload. Default: F5 **/
  static uint reload();

  /** Reload. Default: Ctrl-A **/
  static uint selectAll();

  /** Up. Default: Alt+Up **/
  static uint up();

  /** Back. Default: Alt+Left **/
  static uint back();

  /** Forward. Default: ALT+Right **/
  static uint forward();

  /** Show Menu Bar.  Default: Ctrl-M **/
  static uint showMenubar();

  /**
   * Compare the keys generated by the key event with
   * the value of the integer.
   *
   * If a modifier (Shift, Alt, Ctrl) key is present in
   * @ref QKeyEvent, its sum with the actual key value
   * is used for comparing it with the integer parameter.
   *
   * @param ev the key event to be used in the comparison.
   * @param key the int value to be compared to the key event.
   *
   * @return true if the int value matches the integer representation of the @ref QKeyEvent
   */
  static bool isEqual(QKeyEvent* /*ev*/, int /*key*/);

  /**
   * Return the keybinding for @p accel.
   */
  static uint key(StdAccel accel);

  static QString action(StdAccel);
  static QString description(StdAccel);

  /**
   * @internal
   * Return the hardcoded default keybinding for @p accel.
   * This does not take into account the user's configuration.
   */
  static uint defaultKey(StdAccel accel);
};

#endif
