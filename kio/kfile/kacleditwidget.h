/***************************************************************************
 *   Copyright (C) 2005 by Sean Harmer <sh@rama.homelinux.org>             *
 *                         Till Adam <adam@kde.org>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by  the Free Software Foundation; either version 2 of the   *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef KACLEDITWIDGET_H
#define KACLEDITWIDGET_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef USE_POSIX_ACL

#include <klistview.h>
#include <kacl.h>
#include <kfileitem.h>

class KACLListViewItem;
class KACLListView;
class QPushButton;

class KACLEditWidget : QWidget
{
  Q_OBJECT
public:
  KACLEditWidget( QWidget *parent = 0, const char *name = 0 );
  KACL getACL() const;
  KACL getDefaultACL() const;
  void setACL( const KACL & );
  void setDefaultACL( const KACL & );
  void setAllowDefaults( bool value );
private slots:
  void slotUpdateButtons();

private:
  KACLListView* m_listView;
  QPushButton *m_AddBtn;
  QPushButton *m_EditBtn;
  QPushButton *m_DelBtn;
};


#endif
#endif
