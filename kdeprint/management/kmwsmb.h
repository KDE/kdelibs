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

#ifndef KMWSMB_H
#define KMWSMB_H

#include "kmwizardpage.h"

class SmbView;
class QLineEdit;
class QLabel;

class KMWSmb : public KMWizardPage
{
	Q_OBJECT
public:
	KMWSmb(QWidget *parent = 0, const char *name = 0);

	bool isValid(QString&);
	void updatePrinter(KMPrinter*);
	void initPrinter(KMPrinter*);

protected slots:
	void slotScan();
	void slotAbort();
	void slotPrinterSelected(const QString& work, const QString& server, const QString& printer);

protected:
	SmbView	*m_view;
	QLineEdit	*m_work, *m_server, *m_printer;
	QLabel *m_loginlabel;
};

#endif
