/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>

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
#ifndef KICONVIEW_H
#define KICONVIEW_H

#include <qcursor.h>
#include <qiconview.h>

#include <kdebug.h>

/**
 * This Widget extends the functionality of QIconView to honor the system
 * wide settings for Single Click/Double Click mode, Auto Selection and
 * Change Cursor over Link.
 *
 * There is a new signal executed(). It gets connected to either
 * QIconView::clicked() or QIconView::doubleClicked() depending on the KDE
 * wide Single Click/Double Click settings. It is strongly recomended that
 * you use this signal instead of the above mentioned. This way you don´t
 * need to care about the current settings.
 * If you want to get informed when the user selects something connect to the
 * QIconView::selectionChanged() signal.
 **/
class KIconView : public QIconView
{
  Q_OBJECT

public:
  KIconView( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );

signals:

  /**
   * Gets emitted when the execute operation has been performed.
   */
  void executed( QIconViewItem *item );

protected slots:
 void slotOnItem( QIconViewItem *item );
 void slotOnViewport();

 void slotSettingsChanged();

 /**
  * Auto selection happend.
  */
 void slotAutoSelect();
 void slotExecute( QIconViewItem *item );

 void selch() { debug("SELECTION CHANGED"); }
protected:
  virtual void focusOutEvent( QFocusEvent *fe );
 
  QCursor oldCursor;
  bool m_bUseSingle;
  bool m_bChangeCursorOverItem;

  QIconViewItem* m_pCurrentItem;

  QTimer* m_pAutoSelect;
  int m_autoSelectDelay;

private:
  class KIconViewPrivate;
  KIconViewPrivate *d;
};

#endif
