/*  This file is part of the KDE project
    Copyright (C) 2004,2006 Matthias Kretz <kretz@kde.org>

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

#ifndef BACKENDSELECTION_H
#define BACKENDSELECTION_H

#include "ui_backendselection.h"
#include <QWidget>
#include <QHash>
#include <QString>

#include <kservice.h>

class BackendSelection : public QWidget, private Ui::BackendSelection
{
	Q_OBJECT
	public:
		BackendSelection( QWidget* parent = 0 );

		void load();
		void save();
		void defaults();

	private slots:
		void selectionChanged();
		void up();
		void down();

	signals:
		void changed();

	private:
		QHash<QString, KService::Ptr> m_services;
};

// vim: sw=4 ts=4 noet tw=80
#endif // BACKENDSELECTION_H
