/* This file is part of the KDE project
  Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
#ifndef K3MDI_FOCUS_LIST
#define K3MDI_FOCUS_LIST

#include <qobject.h>
#include <qmap.h>
#include <qwidget.h>
#include <kdelibs_export.h>

class KDE3SUPPORT_EXPORT K3MdiFocusList: public QObject
{
	Q_OBJECT
public:
	K3MdiFocusList( QObject *parent );
	~K3MdiFocusList();
	void addWidgetTree( QWidget* );
	void restore();
protected slots:
	void objectHasBeenDestroyed( QObject* );
private:
	QMap<QWidget*, Qt::FocusPolicy> m_list;

};

#endif 
// kate: space-indent off; tab-width 4; replace-tabs off; indent-mode csands;
