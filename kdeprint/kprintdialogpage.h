/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  $Id:  $
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

#ifndef KPRINTDIALOGPAGE_H
#define KPRINTDIALOGPAGE_H

#include <qwidget.h>
#include <qmap.h>

class KMPrinter;
class DrMain;

class KPrintDialogPage : public QWidget
{
	Q_OBJECT
public:
	KPrintDialogPage(QWidget *parent = 0, const char *name = 0);
	KPrintDialogPage(KMPrinter *pr, DrMain *dr = 0, QWidget *parent = 0, const char *name = 0);
	virtual ~KPrintDialogPage();

	virtual void getOptions(QMap<QString,QString>& opts, bool incldef = false);
	virtual void setOptions(const QMap<QString,QString>& opts);
	virtual bool isValid(QString&);
	int id() const 				{ return m_ID; }
	void setId(int ID)			{ m_ID = ID; }
	QString	title() const 			{ return m_title; }
	void setTitle(const QString& txt)	{ m_title = txt; }
	DrMain* driver() 			{ return m_driver; }
	KMPrinter* printer()			{ return m_printer; }

protected:
	KMPrinter	*m_printer;
	DrMain		*m_driver;
	int 		m_ID;
	QString		m_title;
};

#endif
