/* This file is part of the KDE libraries
    Copyright (C) 1997 Sven Radej <sven.radej@iname.com>
    Copyright (c) 1999 Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
    Copyright (c) 1999 Preston Brown <pbrown@kde.org>

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
#include <qlineedit.h>

/**
 * Use of the class KLineEd is deprecated.  Please use KLineEdit instead.
 */
#define KLineEd KLineEdit

/**
 * This widget has the same behaviour as QLineEdit, but emits
 * signals for two more key-events: @ref completion when Ctrl-D is
 * pressed and  @ref rotation when Ctrl-S is pressed. 
 *
 * Completion can then be connected to a slot that will try and "fill in"
 * the rest of the details, i.e. for URL or filename completion.  Rotation
 * is usually used to rotate through a list of predefined text entries.
 *
 * This class is inspired by Torben Weis' fileentry.cpp for KFM II.
 * @short KDE Line input widget
 */

class KLineEdit : public QLineEdit
{
  Q_OBJECT
        
public:
  KLineEdit ( QWidget *parent=0, const char *name=0 );
  ~KLineEdit ();

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
   
protected slots:
  virtual void mousePressEvent(QMouseEvent *);
  virtual void doCut() { cut(); }
  virtual void doCopy() { copy(); }
  virtual void doPaste() { paste(); }
  virtual void doSelect() { selectAll(); }
  virtual void doClear() { setText(""); }

protected:
  bool eventFilter (QObject *, QEvent *);

private:
  int pmid[5];
  QPopupMenu *pop;
};

#endif
