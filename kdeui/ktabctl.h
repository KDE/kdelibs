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
 * Revision 1.2  1997/07/24 21:06:08  kalle
 * Kalle:
 * KToolBar upgraded to newtoolbar 0.6
 * KTreeList has rubberbanding now
 * Patches for SGI
 *
 * Revision 1.1.1.1  1997/04/13 14:42:43  cvsuser
 * Source imported
 *
 * Revision 1.1.1.1  1997/04/09 00:28:10  cvsuser
 * Sources imported
 *
 * Revision 1.1  1997/03/15 22:40:57  kalle
 * Initial revision
 *
 * Revision 1.2.2.1  1997/01/07 14:41:57  alex
 * release 0.1
 *
 * Revision 1.2  1997/01/07 13:47:09  alex
 * first working release
 *
 * Revision 1.1.1.1  1997/01/07 13:44:53  alex
 * imported
 *
 */

#ifndef KTABCTL_H
#define KTABCTL_H

#include "qwidget.h"
#include "qtabbar.h"
#include "qarray.h"

/// KTabCtl, simple widget for the creation of tabbed window layouts.
/** KTabCtl is very similar to QTabDialog, with the following differences:
 To avoid confusion, the API is almost identical with QTabDialog.
  
  * does not create any button, therefore KTabCtl is not limited to dialog
	boxes. You can use it whereever you want.
	 
 * emits the signal tabSelected(int pagenumber) when the user selects one
   of the tabs. This gives you the chance to update the widget contents
 of a single page. The signal is emitted _before_ the page is shown.
   Very important, if the contents of some widgets on page <a> depend on
	 the contents of some other widgets on page <b>.
*/
class KTabCtl : public QWidget
{
    Q_OBJECT

public:
    KTabCtl(QWidget *parent = 0, const char *name = 0);
   ~KTabCtl();

    void show();
    void setFont(const QFont & font);

    void addTab(QWidget *, const char *);
    bool isTabEnabled(const char * );
    void setTabEnabled(const char *, bool);

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);

signals:
    void tabSelected(int);
    
private slots:
    void showTab(int i);

private:
    void setSizes();
    QRect getChildRect() const;

    QTabBar * tabs;
    QArrayT<QWidget *> pages;
    int bh;
};
#endif
