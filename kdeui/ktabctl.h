/* This file is part of the KDE libraries
    Copyright (C) 1997 Alexander Sanda (alex@darkstar.ping.at)

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
/*
 * $Id$
 *
 * $Log$
 * Revision 1.7  1999/04/19 18:28:17  cschlaeg
 * sizeHint() added; showTab() now also selects the tab; paintEvent updated to
 * match Qt 2.0 style frame
 *
 * Revision 1.6  1999/03/01 23:35:19  kulow
 * CVS_SILENT ported to Qt 2.0
 */

#ifndef KTABCTL_H
#define KTABCTL_H

#include "qwidget.h"
#include "qtabbar.h"
#include "qarray.h"

/** 
 * Tabbed dialog with extended features.
 * KTabCtl is very similar to QTabDialog, with the following differences:
 * 
 * @li To avoid confusion, the API is almost identical with QTabDialog.
 * @li Does not create any buttons, therefore KTabCtl is not limited
 * to dialog boxes. You can use it whereever you want.
 * @li emits the signal tabSelected(int pagenumber) when the user
 * selects one of the tabs. This gives you the chance to update the
 * widget contents of a single page. The signal is emitted _before_ the
 * page is shown.  This is very useful if the contents of some widgets
 * on page A depend on the contents of some other widgets on page B.
 *
 * @author Alexander Sanda (alex@darkstar.ping.at)
 * @version $Id$
*/
class KTabCtl : public QWidget
{
    Q_OBJECT

public:
    KTabCtl(QWidget *parent = 0, const char *name = 0);
   ~KTabCtl();

    void show();
    void setFont(const QFont & font);
    void setTabFont( const QFont &font );

    void addTab(QWidget *, const QString&);
    bool isTabEnabled(const QString& );
    void setTabEnabled(const QString&, bool);
    void setBorder(bool);
    void setShape( QTabBar::Shape shape );
	virtual QSize sizeHint(void);

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);

signals:
    void tabSelected(int);
    
protected slots:
    void showTab(int i);

protected:
    void setSizes();
    QRect getChildRect() const;

    QTabBar * tabs;
    QArray<QWidget *> pages;
    int bh;
    bool blBorder;
};
#endif
