/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef KPSCHEDULEPAGE_H
#define KPSCHEDULEPAGE_H

#include <kprintdialogpage.h>

class QComboBox;
class QLineEdit;
class QButtonGroup;

class KPSchedulePage : public KPrintDialogPage
{
	Q_OBJECT
public:
	KPSchedulePage(QWidget *parent = 0, const char *name = 0);
	~KPSchedulePage();

	void getOptions(QMap<QString,QString>& opts, bool incldef = false);
	void setOptions(const QMap<QString,QString>& opts);
	bool isValid(QString& msg);

protected slots:
	void slotTimeChanged();
	void slotHourChanged();

private:
	QComboBox	*m_time;
	QLineEdit	*m_hh, *m_mm, *m_ss;
	QButtonGroup	*m_group;
	int	m_gmtdiff;
};

#endif
