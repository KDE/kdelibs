/* This file is part of the KDE libraries
    Copyright (C) 1997 Sven Radej (sven.radej@iname.com)

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
// This was taken from filentry.h
// fileentry.h is part of KFM II, by Torben Weis



#ifndef _KLINED_H
#define _KLINED_H
#include <qlined.h>

/**
 * This widget has the same behaviour as QLineEditor, but emits
 * signals for two more key-events: @ref completion when Ctrl-D is
 * pressed and  @ref rotation when Ctrl-S is pressed. This
 * class is inspired by Torben Weis' fileentry.cpp for KFM II
 * @short KDE Line input widget
 */

class KLined : public QLineEdit
{
  Q_OBJECT
        
public:
  KLined ( QWidget *_parent, const char *_name );
  ~KLined ();

  /**
    * This puts cursor at and of string. When using out of toolbar,
    * call this in your slot connected to signal completion.
    */
  void cursorAtEnd();

  signals:

  /**
	* Connect to this signal to receive Ctrl-D
	*/
  void completion ();
    
  /**
	* Connect to this signal to receive Ctrl-S
	*/
  void rotation ();
    
 protected:
     bool eventFilter (QObject *, QEvent *);
 };

#endif
