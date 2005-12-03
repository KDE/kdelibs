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

#ifndef KMDRIVERDBWIDGET_H
#define KMDRIVERDBWIDGET_H

#include <qwidget.h>
#include "kmdbentry.h"

class Q3ListBox;
class QCheckBox;
class QPushButton;

class KMDriverDbWidget : public QWidget
{
	Q_OBJECT
public:
	KMDriverDbWidget(QWidget *parent = 0);
	~KMDriverDbWidget();

	void init();
	void setHaveRaw(bool on);
	void setHaveOther(bool on);
	void setDriver(const QString& manu, const QString& model);

	QString manufacturer();
	QString model();
	QString description()		{ return m_desc; }
	KMDBEntryList* drivers();
	QString driverFile();
	bool isRaw();
	bool isExternal();

protected slots:
	void slotDbLoaded(bool reloaded);
	void slotManufacturerSelected(const QString& name);
	void slotPostscriptToggled(bool);
	void slotOtherClicked();
	void slotError(const QString&);

private:
	Q3ListBox	*m_manu;
	Q3ListBox	*m_model;
	QCheckBox	*m_postscript;
	QCheckBox	*m_raw;
	QPushButton	*m_other;
	QString		m_external;
	QString		m_desc;
	bool		m_valid;
};

inline QString KMDriverDbWidget::driverFile()
{ return m_external; }

inline bool KMDriverDbWidget::isExternal()
{ return !(m_external.isEmpty()); }

#endif
