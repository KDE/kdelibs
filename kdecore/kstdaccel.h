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
  KStdAccel(KConfig* cfg = 0);
  ~KStdAccel();

  /** Open file. Default: Ctrl-o */
  uint open() const;

  /** Create a new document (or whatever). Default: Ctrl-n */
  uint openNew() const;

  /** Close current document. Default: Ctrl-w */
  uint close() const;

  /** Save current document. Default: Ctrl-s */
  uint save() const;

  /** Print current document. Default: Ctrl-p */
  uint print() const;

  /** Quit the program. Default: Ctrl-q */
  uint quit() const;

  /** Cut selected area and store it in the clipboard. Default: Ctrl-x */
  uint cut() const;

  /** Copy selected area into the clipboard. Default: Ctrl-c */
  uint copy() const;

  /** Paste contents of clipboard at mouse/cursor position. Default: Ctrl-v */
  uint paste() const;

  /** Undo last operation. Default: Ctrl-z */
  uint undo() const;

  /** Redo. Default: Ctrl-y */
  uint redo() const;

  /** Find, search. Default: Ctrl-f */
  uint find() const;

  /** Find and replace matches. Default: Ctrl-r */
  uint replace() const;

  /** Toggle insert/overwrite (with visual feedback, e.g. in the statusbar). Default: Insert */
  uint insert() const;

  /** Goto beginning of current line. Default: Home */
  uint home() const;

  /** Goto end of current line. Default: End */
  uint end() const;

  /** Scroll up one page. Default: Prior */
  uint prior() const;

  /** Scroll down one page. Default: Next */
  uint next() const;

  /** Help the user in the current situation. Default: F1 */
  uint help() const;

protected:
  uint readKey(KAccel::StdAccel accel, uint defaultKey=0) const;
  KConfig* config;
};

#endif /*kshortcut_h*/
