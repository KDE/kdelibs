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
// Idea was taken from filentry.h
// fileentry.h is part of KFM II, by Torben Weis

#ifndef _KCOMBO_H
#define _KCOMBO_H

#include <qcombo.h>

/** A new combobox for the KDE project.
  */
class KCombo : public QComboBox
{
  Q_OBJECT

public:
  /**
    *
    * Kcombo is normal writable or readonly ComboBox with two
    * more signals @ref completion (Ctrl-D pressed) and @ref rotation
    * (Ctrl-S pressed).
    * @short KDE combo-box Widget
    * @author sven.radej@iname.com
    */
   
  KCombo ( bool _rw, QWidget *_parent, const char *name );
  ~KCombo ();

  /**
    * This puts cursor at and of string. When using out of toolbar,
    * call this in your slot connected to signal @ref completion
    * or @ref rotation
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
