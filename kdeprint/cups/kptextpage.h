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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef KPTEXTPAGE_H
#define KPTEXTPAGE_H

#include "kprintdialogpage.h"

class DrMain;
class KIntNumInput;
class QButtonGroup;
class QLabel;
class MarginWidget;

class KPTextPage : public KPrintDialogPage
{
	Q_OBJECT
public:
	KPTextPage(DrMain *driver, QWidget *parent = 0, const char *name = 0);
	~KPTextPage();

	void setOptions(const QMap<QString,QString>& opts);
	void getOptions(QMap<QString,QString>& opts, bool incldef = false);

protected slots:
	void slotPrettyChanged(int);
	void slotColumnsChanged(int);

protected:
	void initPageSize(bool landscape);

private:
	KIntNumInput	*m_cpi, *m_lpi, *m_columns;
	QButtonGroup	*m_prettyprint;
	MarginWidget	*m_margin;
	QLabel			*m_prettypix;
	QString		m_currentps;
	bool		m_block;
};

#endif
