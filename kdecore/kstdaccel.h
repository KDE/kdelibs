/* This file is part of the KDE libraries
    Copyright (C) 1997 Stefan Taferner (taferner@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

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

class KConfig;
#include "kaccel.h"

/**
 * Convenient methods for access of the common accelerator keys in
 * the key configuration. These standard keybindings that shall be used
 * in all Kde applications. They will be configurable, so do not hardcode
 * the default behaviour.
 *
 * If you want real configurable keybindings in your applications,
 * please checkout the class KAccel in kaccel.h
 */
class KStdAccel //: public KKeyConfig
{
public:
  KStdAccel();
  ~KStdAccel();

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

  /** Add current page to bookmarks. Default: Ctrl+B */
  static uint addBookmark();

  /** Help the user in the current situation. Default: F1 */
  static uint help();

protected:
  static uint readKey(KAccel::StdAccel accel, uint defaultKey=0);
};

#endif /*kshortcut_h*/
