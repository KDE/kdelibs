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

#include <qkeycode.h>
#include "kconfig.h"
#include "kaccel.h"
// #include <kkeyconf.h>

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
  uint open(void) const;

  /** Create a new document (or whatever). Default: Ctrl-n */
  uint openNew(void) const;

  /** Close current document. Default: Ctrl-w */
  uint close(void) const;

  /** Save current document. Default: Ctrl-s */
  uint save(void) const;

  /** Print current document. Default: Ctrl-p */
  uint print(void) const;

  /** Quit the program. Default: Ctrl-q */
  uint quit(void) const;

  /** Cut selected area and store it in the clipboard. Default: Ctrl-x */
  uint cut(void) const;

  /** Copy selected area into the clipboard. Default: Ctrl-c */
  uint copy(void) const;

  /** Paste contents of clipboard at mouse/cursor position. Default: Ctrl-v */
  uint paste(void) const;

  /** Undo last operation. Default: Ctrl-z */
  uint undo(void) const;

  /** Redo. Default: Ctrl-y */
  uint redo(void) const;

  /** Find, search. Default: Ctrl-f */
  uint find(void) const;

  /** Find and replace matches. Default: Ctrl-r */
  uint replace(void) const;

  /** Toggle insert/overwrite (with visual feedback, e.g. in the statusbar). Default: Insert */
  uint insert(void) const;

  /** Goto beginning of current line. Default: Home */
  uint home(void) const;

  /** Goto end of current line. Default: End */
  uint end(void) const;

  /** Scroll up one page. Default: Prior */
  uint prior(void) const;

  /** Scroll down one page. Default: Next */
  uint next(void) const;

  /** Help the user in the current situation. Default: F1 */
  uint help(void) const;

protected:
  uint readKey(KAccel::StdAccel accel, uint defaultKey=0) const;
  KConfig* config;
};

#endif /*kshortcut_h*/
