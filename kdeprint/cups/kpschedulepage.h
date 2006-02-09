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

#ifndef KPSCHEDULEPAGE_H
#define KPSCHEDULEPAGE_H

#include <kprintdialogpage.h>

class QComboBox;
class QDateTimeEdit;
class QLineEdit;
class KIntNumInput;

class KPSchedulePage : public KPrintDialogPage
{
	Q_OBJECT
public:
	KPSchedulePage(QWidget *parent = 0);
	~KPSchedulePage();

	void getOptions(QMap<QString,QString>& opts, bool incldef = false);
	void setOptions(const QMap<QString,QString>& opts);
	bool isValid(QString& msg);

protected Q_SLOTS:
	void slotTimeChanged();

private:
	QComboBox	*m_time;
	QDateTimeEdit	*m_tedit;
	QLineEdit	*m_billing, *m_pagelabel;
	KIntNumInput	*m_priority;
	int	m_gmtdiff;
};

#endif
