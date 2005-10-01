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

#ifndef KMWIZARDPAGE_H
#define KMWIZARDPAGE_H

#include <qwidget.h>

#include <kdelibs_export.h>

class KMPrinter;

class KDEPRINT_EXPORT KMWizardPage : public QWidget
{
	Q_OBJECT
public:
	KMWizardPage(QWidget *parent = 0, const char *name = 0);
	virtual ~KMWizardPage();

	int id() const 			{ return m_ID; }
	const QString& title() const 	{ return m_title; }
	int nextPage() const 		{ return m_nextpage; }
	void setNextPage(int p)		{ m_nextpage = p; }
	virtual bool isValid(QString&);
	virtual void initPrinter(KMPrinter*);
	virtual void updatePrinter(KMPrinter*);
	bool needsInitOnBack() 		{ return m_needsinitonback; }

protected:
	QString	m_title;
	int	m_ID;
	int	m_nextpage;
	bool 	m_needsinitonback;
};

#endif
