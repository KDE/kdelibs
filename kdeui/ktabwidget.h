/* This file is part of the KDE libraries

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

#ifndef KTABWIDGET_H
#define KTABWIDGET_H

#include <qtabwidget.h>
#include "ktabbar.h"

class KTabBar;
class KTabWidgetPrivate;

class KTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    KTabWidget( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );

private:
    KTabBar *m_pTabBar;

    KTabWidgetPrivate *d;
};

#endif
