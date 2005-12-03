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

#ifndef KMPROPWIDGET_H
#define KMPROPWIDGET_H

#include <qwidget.h>

#include <kdelibs_export.h>

class KMPrinter;
class KMWizard;

class KDEPRINT_EXPORT KMPropWidget : public QWidget
{
	Q_OBJECT
public:
	KMPropWidget(QWidget *parent = 0);
	virtual ~KMPropWidget();

	virtual void setPrinter(KMPrinter*);
	void setPrinterBase(KMPrinter*);
	QString pixmap() const 	{ return m_pixmap; }
	QString title() const 	{ return m_title; }
	QString header() const 	{ return m_header; }
	bool canChange() const 	{ return m_canchange; }

signals:
	void enable(bool);
	void enableChange(bool);

public slots:
	void slotChange();

protected:
	virtual int requestChange();
	virtual void configureWizard(KMWizard*);

protected:
	QString		m_pixmap;
	QString		m_title;
	QString		m_header;
	KMPrinter	*m_printer;
	bool 		m_canchange;
};

#endif
