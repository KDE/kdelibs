/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef KMCONFIGPAGE_H
#define KMCONFIGPAGE_H

#include <qwidget.h>

#include <kdelibs_export.h>

class KConfig;

class KDEPRINT_MANAGEMENT_EXPORT KMConfigPage : public QWidget
{
	Q_OBJECT
public:
	KMConfigPage(QWidget *parent = 0);

	virtual void loadConfig(KConfig*);
	virtual void saveConfig(KConfig*);

	QString pageName() const 	{ return m_name; }
	QString pageHeader() const 	{ return m_header; }
	QString pagePixmap() const 	{ return m_pixmap; }

protected:
	void setPageName(const QString& s)	{ m_name = s; }
	void setPageHeader(const QString& s)	{ m_header = s; }
	void setPagePixmap(const QString& s)	{ m_pixmap = s; }

protected:
	QString	m_name;
	QString	m_header;
	QString	m_pixmap;
};

#endif
