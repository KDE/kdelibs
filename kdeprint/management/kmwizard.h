/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  $Id$
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

#ifndef KMWIZARD_H
#define KMWIZARD_H

#include <qdialog.h>
#include <qintdict.h>
#include <qvaluestack.h>

class QWidgetStack;
class QLabel;
class QPushButton;
class KMWizardPage;
class KMPrinter;
class KMWBackend;
class SidePixmap;

class KMWizard : public QDialog
{
	Q_OBJECT
public:
	enum PageType {
		Start   = 0,
		End,
		Error,
		Backend,
		Driver,
		File,
		SMB,
		TCP,
		Local,
		LPD,
		IPP,
		IPPSelect,
		Class,
		Password,
		DriverTest,
		DriverSelect,
		Name,
		Banners,
		Custom  = 100
	};

	KMWizard(QWidget *parent = 0, const char *name = 0);
	~KMWizard();

	void configure(int start, int end, bool inclusive = true);
	void setCurrentPage(int ID, bool back = false);
	void setPrinter(KMPrinter*);
	KMPrinter* printer()		{ return m_printer; }
	void addPage(KMWizardPage*);
	KMWBackend* backendPage()	{ return m_backend; }
	void setNextPage(int page, int next);

public slots:
	void enableWizard();
	void disableWizard();

protected slots:
	void slotNext();
	void slotPrev();
	void slotHelp();

private:
	QIntDict<KMWizardPage>	m_pagepool;
	QValueStack<int>	m_pagestack;

	QWidgetStack		*m_stack;
	QLabel			*m_title;
	QPushButton		*m_next, *m_prev;
	int			m_start, m_end;
	bool			m_inclusive;
	KMPrinter		*m_printer;

	// backend page
	KMWBackend		*m_backend;

	// side pixmap
	SidePixmap		*m_side;
};

#endif
