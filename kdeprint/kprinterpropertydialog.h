/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
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

#ifndef KPRINTERPROPERTYDIALOG_H
#define KPRINTERPROPERTYDIALOG_H

#include <kdialogbase.h>
#include <qlist.h>
#include <qmap.h>

class KMPrinter;
class KPrintDialogPage;
class DrMain;
class QTabWidget;

class KDEPRINT_EXPORT KPrinterPropertyDialog : public KDialogBase
{
	Q_OBJECT
public:
	KPrinterPropertyDialog(KMPrinter *printer, QWidget *parent = 0, const char *name = 0);
	~KPrinterPropertyDialog();

	KMPrinter* printer() 		{ return m_printer; }
	DrMain* driver()		{ return m_driver; }
	void setDriver(DrMain* d) 	{ if (!m_driver) m_driver = d; }
	void addPage(KPrintDialogPage*);
	void setOptions(const QMap<QString,QString>&);
	void getOptions(QMap<QString,QString>&, bool = false);
	void enableSaveButton(bool);

	static void setupPrinter(KMPrinter *printer, QWidget *parent);

protected:
	bool synchronize();
	void collectOptions(QMap<QString,QString>& opts, bool incldef = false);

protected Q_SLOTS:
	void slotCurrentChanged(QWidget*);
	void slotUser1();
	void slotOk();

protected:
	KMPrinter		*m_printer;
	DrMain			*m_driver;
	QList<KPrintDialogPage*>	m_pages;
	KPrintDialogPage	*m_current;
	QMap<QString,QString>	m_options;
	QTabWidget		*m_tw;
	QPushButton		*m_save;
};

#endif
