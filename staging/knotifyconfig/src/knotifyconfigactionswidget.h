/*  This file is part of the KDE project
   Copyright (C) 2005-2007 by Olivier Goffart <ogoffart at kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

 */

#ifndef KNOTIFYCONFIGACTIONSWIDGET_H
#define KNOTIFYCONFIGACTIONSWIDGET_H

#include <QWidget>
#include "ui_knotifyconfigactionswidgetbase.h"

class KNotifyConfigElement;

/**
 * Represent the config for an event
 *  @internal
 *  @author Olivier Goffart <ogoffart @ kde.org>
 */
class KNotifyConfigActionsWidget : public QWidget
{
	Q_OBJECT
	public:
		KNotifyConfigActionsWidget( QWidget *parent);
		~KNotifyConfigActionsWidget() {}

		void setConfigElement( KNotifyConfigElement *config );
		void save( KNotifyConfigElement *config );
	Q_SIGNALS:
		void changed();
	private Q_SLOTS:
		void slotPlay();
		void slotKTTSComboChanged();
	private:
		Ui::KNotifyConfigActionsWidgetBase m_ui;
};

#endif // KNOTIFYCONFIGACTIONSWIDGET_H
